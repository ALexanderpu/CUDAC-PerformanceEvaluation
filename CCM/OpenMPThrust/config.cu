#include "csv.hpp"
#include "ccm.hpp"
#include "config.hpp"
#include <string>
using namespace std;
int main(int argc, char *argv[])
{
    // test accuracy and function of ccm
    size_t E = 2;
    size_t tau = 3;
    size_t lib_size = 4;
    int num_samples = 11;
    vector<float > result = {1,2,3,4,5};
    string output = "/home/bo/cloud/CCM-Parralization/Result";
    string output_file = "E_" + to_string(E) + "_tau_" + to_string(tau) + "_l_" + to_string(lib_size) + "_numsamples_" + to_string(num_samples) + ".csv";
    string output_path = output + "/" + output_file;
    cout << "full output file path: " << output_path << endl;
    dump_csv(output_path, result, E, tau, lib_size);
}
