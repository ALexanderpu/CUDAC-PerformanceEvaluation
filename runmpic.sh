#!/bin/bash
echo "mpi will not use cuda lib: please remove rank_matrix_gpu in global.h"


echo "compiling the program for MPI distributed version of ccm"

mpicxx -fopenmp -std=c++11 -o ./MPIVersion/MPIInterface ./CCM/MPIInterface.cpp

echo "running mpi distributed version of ccm"
mpirun -np 3 --hosts 10.80.64.110,10.80.64.35,10.80.64.53 ./MPIVersion/MPIInterface