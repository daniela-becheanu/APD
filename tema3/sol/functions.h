#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "mpi.h"
#include "tema3.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <fstream>
using namespace std;

int *read_input_file(int rank, int &size);
char *create_topology_string(topology t);
int *create_array(int initial_dim, int aux_dim);

#endif