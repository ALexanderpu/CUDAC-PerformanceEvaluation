#include "mpi.h"
#include "include/ccm.hpp"
#include "include/config.hpp"
#include "include/csv.hpp"

using namespace std;

struct IncGenerator {
    int current_, interval_;
    IncGenerator (int start, int interval) : current_(start),interval_(interval) {}
    int operator() () { return current_ += interval_; }
};

int main(int argc, char **argv){
    string file = "/home/bo/cloud/CCM-Parralization/ccm.cfg";
    ConfigReader cr;
    int num_samples;
    std::vector<int> EArr, tauArr, LArr;
    string input, output;
    vector<float > observations, targets;
    int num_vectors;

    // count the time
    double start_time, end_time;
    int namelen;
    char processor_name[MPI_MAX_PROCESSOR_NAME];


    std::vector<int> combinationEArr, combinationtauArr, combinationLArr;
    int elements_per_proc;

    int my_id, num_procs, num_tasks;
    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI::Get_processor_name(processor_name, namelen);
    cout << "Process ID " << my_id << " is on " << processor_name << endl;

    if(my_id == 0){
        // read settings from local file
        
        try{
            // parse config file: /home/bo/cloud/CCM-Parralization/ccm.cfg
            cr.read_file(file);

            // read parameters and scatter to other processor
            string E = cr.get_string("parameters", "E");
            string tau = cr.get_string("parameters", "tau");

            // parse to int vector for parameter: tau, E
            std::stringstream Ess(E), tauss(tau);
            int i;
            while(Ess >> i){
                EArr.push_back(i);
                if(Ess.peek() == ',')
                    Ess.ignore();
            }
            while(tauss >> i){
                tauArr.push_back(i);
                if(tauss.peek() == ',')
                    tauss.ignore();
            }

            num_samples = stoi(cr.get_string("parameters", "num_samples"));

            // parse to int for parameter: L
            int LStart = stoi(cr.get_string("parameters", "LStart"));
            int LEnd = stoi(cr.get_string("parameters", "LEnd"));
            int LInterval = stoi(cr.get_string("parameters", "LInterval"));
            size_t Lsize = (LEnd-LStart)/LInterval + 1;
            
            LArr.assign(Lsize, 0);
            IncGenerator g (LStart-LInterval, LInterval);
            std::generate(LArr.begin(), LArr.end(), g);

            // read csv file for observations and targets to broadcast to other processor
            string input = cr.get_string("paths", "input");
            string output = cr.get_string("paths", "output");
            
            std::tie(observations, targets) = parse_csv(input);
            num_vectors = observations.size();
        }catch(int e){
            cout << "loading data and parameter error" <<endl;
            return 1;
        }

        if(observations.size() != targets.size()){
            cout << "input sequence length not match" << endl;
            return 1;
        }
        /*
        num_vectors = 5;
        
        observations.assign(num_vectors, 3);
        targets.assign(num_vectors, 5);
        */
       // how to scatter the parameter settings, E tau, L to assign task to each processor?
       num_tasks = tauArr.size()*EArr.size()*LArr.size();
       elements_per_proc = num_tasks / num_procs; // should be dividable 

       for(auto e: EArr){
           for(int ele = 0; ele < tauArr.size()*LArr.size(); ele++){
                combinationEArr.push_back(ele);
           }
           
           for(auto tau: tauArr){
                for(int ele = 0; ele < LArr.size(); ele++){
                   combinationtauArr.push_back(ele);
                }   
                for(auto l: LArr){
                   combinationLArr.push_back(l);
                }
           }
       }
    }

    MPI_Bcast(&num_vectors, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&num_samples, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&num_tasks, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&elements_per_proc, 1, MPI_INT, 0, MPI_COMM_WORLD);


    if(my_id != 0){
        observations.resize(num_vectors); 
        targets.resize(num_vectors);
        combinationEArr.resize(num_tasks);
        combinationtauArr.resize(num_tasks);
        combinationLArr.resize(num_tasks);
    }

    MPI_Bcast(&observations[0], observations.size(), MPI_FLOAT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&targets[0], targets.size(), MPI_FLOAT, 0, MPI_COMM_WORLD);
    
    MPI_Bcast(&combinationEArr[0], combinationEArr.size(), MPI_FLOAT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&combinationtauArr[0], combinationtauArr.size(), MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&combinationLArr[0], combinationLArr.size(), MPI_FLOAT, 0, MPI_COMM_WORLD);

    // call ccm here
    int start_index = my_id*elements_per_proc;
    int end_index = start_index + elements_per_proc-1;

    for(int i = start_index; i <= end_index; i++){
        
        vector<float> result = ccm(observations, targets, combinationEArr[i], combinationtauArr[i], combinationLArr[i], num_samples);
        cout << "task " << i << " has the first result: " << result[0] << endl;
        // better to save result into csv here
    }
    
    
    // finally gather the result
    MPI_Finalize();
    return 0;
}