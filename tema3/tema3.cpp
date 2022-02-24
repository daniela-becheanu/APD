#include "mpi.h"
#include "tema3.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <fstream>
using namespace std;

std::vector<int> readInputFiles(int rank) {
    int workerNo, worker;
    vector<int> workers;
    string filename = "cluster" + to_string(rank) + ".txt";
    ifstream inputFile;
    inputFile.open(filename);

    inputFile >> workerNo;

    for (int i = 0; i < workerNo; ++i) {
        inputFile >> worker;
        workers.push_back(worker);
    }
    return workers;
}

int main (int argc, char *argv[])
{
    int old_size, new_size;
    int old_rank, new_rank;
    int recv_rank;
    MPI_Comm custom_group;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &old_size); // Total number of processes.
    MPI_Comm_rank(MPI_COMM_WORLD, &old_rank); // The current process ID / Rank.

    topology t;

    // if (old_rank == 0 || old_rank == 1 || old_rank == 2) {
        
    // }

    switch(old_rank) {
        case 0:
            t.cluster0 = readInputFiles(old_rank);
            break;
        case 1:
            t.cluster1 = readInputFiles(old_rank);
            break;
        case 2:
            t.cluster2 = readInputFiles(old_rank);
            break;
        default:
    }

    MPI_Finalize();

}