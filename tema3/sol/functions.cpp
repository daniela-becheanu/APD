#include "mpi.h"
#include "tema3.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <fstream>
using namespace std;

// function that reads from the input file corresponding to each coordinator
// and returns the array with the indexes of it
int *read_input_file(int rank, int &size) {
    int worker;
    string filename = "cluster" + to_string(rank) + ".txt";

    ifstream inputFile;
    inputFile.open(filename);

    inputFile >> size;

    int *workers = (int *)calloc(size, sizeof(int)); 

    for (int i = 0; i < size; ++i) {
        inputFile >> worker;
        workers[i] = worker;
    }

    inputFile.close();
    return workers;
}

// create the string corresponding to a topology
char *create_topology_string(topology t) {
    string auxStr = "0:";
    int i;
    char *topologyStr = (char *)calloc(4 * (t.size0 + t.size1 + t.size2), sizeof(char));

    for (i = 0; i < t.size0 - 1; ++i) {
        auxStr.append(to_string(t.cluster0[i]).c_str());
        auxStr.append(",");
    }
    auxStr.append(to_string(t.cluster0[i]).c_str());

    auxStr.append(" 1:");
    for (i = 0; i < t.size1 - 1; ++i) {
        auxStr.append(to_string(t.cluster1[i]).c_str());
        auxStr.append(",");
    }
    auxStr.append(to_string(t.cluster1[i]).c_str());

    auxStr.append(" 2:");
    for (i = 0; i < t.size2 - 1; ++i) {
        auxStr.append(to_string(t.cluster2[i]).c_str());
        auxStr.append(",");
    }
    auxStr.append(to_string(t.cluster2[i]).c_str());
    strcpy(topologyStr, auxStr.c_str());

    return topologyStr;
}

// initialize the array
int *create_array(int initial_dim, int aux_dim) {
    int *v = (int *)calloc(aux_dim, sizeof(int));
    for (int i = 0; i < initial_dim; ++i) {
        v[i] = i;
    }
    return v;
}