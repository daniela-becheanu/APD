#ifndef TEMA3_H
#define TEMA3_H

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <fstream>

#define TAG_START_INDEX_BACK 10
#define TAG_START_INDEX_BACK_COORD_1 111
#define TAG_START_INDEX_BACK_COORD_2 112
#define TAG_PORTIONS_BACK 20
#define TAG_PORTIONS_BACK_COORD_1 211
#define TAG_PORTIONS_BACK_COORD_2 212
#define TAG_LENGTH_COORD_1 1001
#define TAG_LENGTH_COORD_2 1002
#define TAG_LENGTH_BACK_COORD_1 1031
#define TAG_LENGTH_BACK_COORD_2 1032
#define TAG_START_INDEX_COORD_1 1011
#define TAG_START_INDEX_COORD_2 1012
#define TAG_V_PORTION_COORD_1 1021
#define TAG_V_PORTION_COORD_2 1022
#define TAG_LENGTH_WORKER 110
#define TAG_START_INDEX_WORKER 111
#define TAG_V_PORTION_WORKER 112
#define TAG_TOPOLOGY_LEN 30
#define TAG_TOPOLOGY_STR 40

struct topology {
    int *cluster0;
    int *cluster1;
    int *cluster2;
    int size0;
    int size1;
    int size2;
};

#endif