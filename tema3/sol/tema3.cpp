#include "mpi.h"
#include "tema3.h"
#include "functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <fstream>
using namespace std;

int main (int argc, char *argv[])
{
    int size, rank, v_portion_length, start_index, topology_len, initial_dim;
    MPI_Status status;
    topology t;
    char *topology_str;
    int *v_portion;
    int *v;
    bool communication_error = atoi(argv[2]) == 1;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    switch(rank) {
        case 0:
        {   
            // read the input file for coordinator 0
            t.cluster0 = read_input_file(rank, t.size0);

            // send the size of th current coordiantor to coordinator 2
            MPI_Send(&t.size0, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
            cout << "M(0,2)\n";

            // receive size1 and size2 from neighbor 2
            MPI_Recv(&t.size2, 1, MPI_INT, 2, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&t.size1, 1, MPI_INT, 2, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // allocate memory for cluster1 and cluster2
            t.cluster1 = (int *)calloc(t.size1, sizeof(int));
            t.cluster2 = (int *)calloc(t.size2, sizeof(int));

            // send th current cluster to neighbor 2, in order for it to be sent to coordinator 1 by it
            MPI_Send(t.cluster0, t.size0, MPI_INT, 2, 0, MPI_COMM_WORLD);
            cout << "M(0,2)\n";

            // receive the other coordinators' clusters from neigbor 2
            MPI_Recv(t.cluster2, t.size2, MPI_INT, 2, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(t.cluster1, t.size1, MPI_INT, 2, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // create the topology in string format (char*) and print it to stdout
            topology_str = create_topology_string(t);
            cout << "0 -> " << topology_str << endl;

            topology_len = strlen(topology_str) + 1;
            
            // send the length of the string, then the topology to the workers corresponding to this coordinator
            for (int i = 0; i < t.size0; ++i) {
                MPI_Send(&topology_len, 1, MPI_INT, t.cluster0[i], TAG_TOPOLOGY_LEN, MPI_COMM_WORLD);
                MPI_Send(topology_str, topology_len, MPI_CHAR, t.cluster0[i], TAG_TOPOLOGY_STR, MPI_COMM_WORLD);
                cout << "M(0," << t.cluster0[i] << ")\nM(0," << t.cluster0[i] << ")\n";
            }

            initial_dim = atoi(argv[1]);

            int workers_no = t.size0 + t.size1 + t.size2;
            int aux_dim = initial_dim;

            // for filling the array with zeros at the end in order to have uniform distribution
            while (aux_dim % workers_no != 0) {
                ++aux_dim;
            }

            v = create_array(initial_dim, aux_dim);
            
            // send v_portion_length, start_index then the portion of array for cluster1 and cluster2 to coordinator 2
            v_portion_length = aux_dim / workers_no * t.size1;
            MPI_Send(&v_portion_length, 1, MPI_INT, 2, TAG_LENGTH_COORD_1, MPI_COMM_WORLD);
            start_index = aux_dim / workers_no * t.size0;
            MPI_Send(&start_index, 1, MPI_INT, 2, TAG_START_INDEX_COORD_1, MPI_COMM_WORLD);
            MPI_Send(v + start_index, v_portion_length, MPI_INT, 2, TAG_V_PORTION_COORD_1, MPI_COMM_WORLD);
            cout << "M(0,2)\nM(0,2)\nM(0,2)\n";

            v_portion_length = aux_dim / workers_no * t.size2;
            MPI_Send(&v_portion_length, 1, MPI_INT, 2, TAG_LENGTH_COORD_2, MPI_COMM_WORLD);
            start_index = aux_dim / workers_no * (t.size0 + t.size1);
            MPI_Send(&start_index, 1, MPI_INT, 2, TAG_START_INDEX_COORD_2, MPI_COMM_WORLD);
            MPI_Send(v + start_index, v_portion_length, MPI_INT, 2, TAG_V_PORTION_COORD_2, MPI_COMM_WORLD);
            cout << "M(0,2)\nM(0,2)\nM(0,2)\n";

            // reset v_portion_length and start_index for the current cluster
            v_portion_length = aux_dim / workers_no * t.size0;
            start_index = 0;

            int v_portion_length_copy = v_portion_length;
            v_portion_length /= t.size0;

            // send start_index, v_portion_length and the portion of array corresponding to each worker in the current cluster
            for (int i = 0; i < t.size0; ++i) {
                MPI_Send(&start_index, 1, MPI_INT, t.cluster0[i], TAG_START_INDEX_WORKER, MPI_COMM_WORLD);
                MPI_Send(&v_portion_length, 1, MPI_INT, t.cluster0[i], TAG_LENGTH_WORKER, MPI_COMM_WORLD);
                MPI_Send(v + start_index, v_portion_length, MPI_INT, t.cluster0[i], TAG_V_PORTION_WORKER, MPI_COMM_WORLD);
                cout << "M(0," << t.cluster0[i] << ")\nM(0," << t.cluster0[i] << ")\nM(0," << t.cluster0[i] << ")\n";

                start_index += v_portion_length;
            }

            for (int i = 0; i < t.size0; ++i) {
                MPI_Recv(&start_index, 1, MPI_INT, MPI_ANY_SOURCE, TAG_START_INDEX_BACK, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Recv(v + start_index, v_portion_length, MPI_INT, MPI_ANY_SOURCE, start_index, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }

            // reset v_portion_length and start_index for the current cluster
            v_portion_length = v_portion_length_copy;
            start_index = 0;

            // receive from coordinator 2 start_index, v_portion_length and the part in the array corresponding to each cluster
            MPI_Recv(&start_index, 1, MPI_INT, 2, TAG_START_INDEX_BACK_COORD_1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&v_portion_length, 1, MPI_INT, 2, TAG_LENGTH_BACK_COORD_1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(v + start_index, v_portion_length, MPI_INT, 2, TAG_PORTIONS_BACK_COORD_1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            MPI_Recv(&start_index, 1, MPI_INT, 2, TAG_START_INDEX_BACK_COORD_2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&v_portion_length, 1, MPI_INT, 2, TAG_LENGTH_BACK_COORD_2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(v + start_index, v_portion_length, MPI_INT, 2, TAG_PORTIONS_BACK_COORD_2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            break;
        }
        case 1:
        {
            // read the input file for coordinator 1
            t.cluster1 = read_input_file(rank, t.size1);

            // send the size of th current coordiantor to coordinator 2
            MPI_Send(&t.size1, 1, MPI_INT, 2, 1, MPI_COMM_WORLD);
            cout << "M(1,2)\n";

            // receive size2 and size0 from neighbor 2
            MPI_Recv(&t.size2, 1, MPI_INT, 2, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&t.size0, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // allocate memory for cluster1 and cluster2
            t.cluster0 = (int *)calloc(t.size0, sizeof(int));
            t.cluster2 = (int *)calloc(t.size2, sizeof(int));
            
            // send th current cluster to neighbor 2, in order for it to be sent to coordinator 1 by it
            MPI_Send(t.cluster1, t.size1, MPI_INT, 2, 1, MPI_COMM_WORLD);
            cout << "M(1,2)\n";

            // receive the other coordinators' clusters from neigbor 2
            MPI_Recv(t.cluster2, t.size2, MPI_INT, 2, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(t.cluster0, t.size0, MPI_INT, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // create the topology in string format (char*) and print it to stdout
            topology_str = create_topology_string(t);
            cout << "1 -> " << topology_str << endl;

            topology_len = strlen(topology_str) + 1;

            // send the length of the string, then the topology to the workers corresponding to this coordinator
            for (int i = 0; i < t.size1; ++i) {
                MPI_Send(&topology_len, 1, MPI_INT, t.cluster1[i], TAG_TOPOLOGY_LEN, MPI_COMM_WORLD);
                MPI_Send(topology_str, topology_len, MPI_CHAR, t.cluster1[i], TAG_TOPOLOGY_STR, MPI_COMM_WORLD);
                cout << "M(1," << t.cluster1[i] << ")\nM(1," << t.cluster1[i] << ")\n";
            }

            // receive v_portion_length, start_index and v_portion for the current cluster
            MPI_Recv(&v_portion_length, 1, MPI_INT, 2, TAG_LENGTH_COORD_1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&start_index, 1, MPI_INT, 2, TAG_START_INDEX_COORD_1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            v_portion = (int *)calloc(v_portion_length, sizeof(int));
            MPI_Recv(v_portion, v_portion_length, MPI_INT, 2, TAG_V_PORTION_COORD_1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            int v_portion_length_copy = v_portion_length;
            int start_index_copy = start_index;
            v_portion_length /= t.size1;
            start_index = 0;

            // send start_index, v_portion_length and the portion of array corresponding to each worker in the current cluster
            for (int i = 0; i < t.size1; ++i) {
                MPI_Send(&start_index, 1, MPI_INT, t.cluster1[i], TAG_START_INDEX_WORKER, MPI_COMM_WORLD);
                MPI_Send(&v_portion_length, 1, MPI_INT, t.cluster1[i], TAG_LENGTH_WORKER, MPI_COMM_WORLD);
                MPI_Send(v_portion + start_index, v_portion_length, MPI_INT, t.cluster1[i], TAG_V_PORTION_WORKER, MPI_COMM_WORLD);
                cout << "M(1," << t.cluster1[i] << ")\nM(1," << t.cluster1[i] << ")\nM(1," << t.cluster1[i] << ")\n";

                start_index += v_portion_length;
            }

            // receive from the corresponding workers the portion from the array multiplied by 2
            for (int i = 0; i < t.size1; ++i) {
                MPI_Recv(&start_index, 1, MPI_INT, MPI_ANY_SOURCE, TAG_START_INDEX_BACK, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Recv(v_portion + start_index, v_portion_length, MPI_INT, MPI_ANY_SOURCE, start_index, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }

            // reset v_portion_length and start_index for the current cluster
            v_portion_length = v_portion_length_copy;
            start_index = start_index_copy;

            // send to coordinator 2 start_index, v_portion_length and v_portion, for it to send them furter to coordinator 1
            MPI_Send(&start_index, 1, MPI_INT, 2, TAG_START_INDEX_BACK_COORD_1, MPI_COMM_WORLD);
            MPI_Send(&v_portion_length, 1, MPI_INT, 2, TAG_LENGTH_BACK_COORD_1, MPI_COMM_WORLD);
            MPI_Send(v_portion, v_portion_length, MPI_INT, 2, TAG_PORTIONS_BACK_COORD_1, MPI_COMM_WORLD);
            cout << "M(1,2)\nM(1,2)\nM(1,2)\n";
            break;
        }

        case 2:
        {
            // read the input file for coordinator 2
            t.cluster2 = read_input_file(rank, t.size2);

            // send size2 to cluster0 and cluster1
            MPI_Send(&t.size2, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
            MPI_Send(&t.size2, 1, MPI_INT, 1, 2, MPI_COMM_WORLD);
            cout << "M(2,0)\nM(2,1)\n";

            // receive sizes from neighbors
            MPI_Recv(&t.size0, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&t.size1, 1, MPI_INT, 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // send size0 to cluser1 and size1 to cluster0
            MPI_Send(&t.size0, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
            MPI_Send(&t.size1, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
            cout << "M(2,1)\nM(2,0)\n";

            // allocate memory for cluster0 and cluster1
            t.cluster0 = (int *)calloc(t.size0, sizeof(int));
            t.cluster1 = (int *)calloc(t.size1, sizeof(int));

            // send the current cluster to coordinator 1 and 2
            MPI_Send(t.cluster2, t.size2, MPI_INT, 0, 2, MPI_COMM_WORLD);
            MPI_Send(t.cluster2, t.size2, MPI_INT, 1, 2, MPI_COMM_WORLD);
            cout << "M(2,0)\nM(2,1)\n";

            // receive cluster0 and cluster1
            MPI_Recv(t.cluster0, t.size0, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(t.cluster1, t.size1, MPI_INT, 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // send to coordinator 0 cluster1 and to coordinator 1 cluster0
            MPI_Send(t.cluster0, t.size0, MPI_INT, 1, 0, MPI_COMM_WORLD);
            MPI_Send(t.cluster1, t.size1, MPI_INT, 0, 1, MPI_COMM_WORLD);
            cout << "M(2,1)\nM(2,0)\n";

            // create the topology in string format (char*) and print it to stdout
            topology_str = create_topology_string(t);
            cout << "2 -> " << topology_str << endl;

            topology_len = strlen(topology_str) + 1;
            
            // send the length of the string, then the topology to the workers corresponding to this coordinator
            for (int i = 0; i < t.size2; ++i) {
                MPI_Send(&topology_len, 1, MPI_INT, t.cluster2[i], TAG_TOPOLOGY_LEN, MPI_COMM_WORLD);
                MPI_Send(topology_str, topology_len, MPI_CHAR, t.cluster2[i], TAG_TOPOLOGY_STR, MPI_COMM_WORLD);
                cout << "M(2," << t.cluster2[i] << ")\nM(2," << t.cluster2[i] << ")\nM(2," << t.cluster2[i] << ")\n";
            }

            // receive v_portion_length, start_index and v_portion for cluster1, the send them too coordinator 1
            MPI_Recv(&v_portion_length, 1, MPI_INT, 0, TAG_LENGTH_COORD_1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&start_index, 1, MPI_INT, 0, TAG_START_INDEX_COORD_1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            v_portion = (int *)calloc(v_portion_length, sizeof(int));
            MPI_Recv(v_portion, v_portion_length, MPI_INT, 0, TAG_V_PORTION_COORD_1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            MPI_Send(&v_portion_length, 1, MPI_INT, 1, TAG_LENGTH_COORD_1, MPI_COMM_WORLD);
            MPI_Send(&start_index, 1, MPI_INT, 1, TAG_START_INDEX_COORD_1, MPI_COMM_WORLD);
            MPI_Send(v_portion, v_portion_length, MPI_INT, 1, TAG_V_PORTION_COORD_1, MPI_COMM_WORLD);
            cout << "M(2,1)\nM(2,1)\nM(2,1)\n";

            // receive v_portion_length, start_index and v_portion for the current cluster
            MPI_Recv(&v_portion_length, 1, MPI_INT, 0, TAG_LENGTH_COORD_2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&start_index, 1, MPI_INT, 0, TAG_START_INDEX_COORD_2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            v_portion = (int *)realloc(v_portion, v_portion_length * sizeof(int));
            MPI_Recv(v_portion, v_portion_length, MPI_INT, 0, TAG_V_PORTION_COORD_2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            int v_portion_length_copy = v_portion_length;
            int start_index_copy = start_index;
            v_portion_length /= t.size2;
            start_index = 0;
            
            // send start_index, v_portion_length and the portion of array corresponding to each worker in the current cluster
            for (int i = 0; i < t.size2; ++i) {
                MPI_Send(&start_index, 1, MPI_INT, t.cluster2[i], TAG_START_INDEX_WORKER, MPI_COMM_WORLD);
                MPI_Send(&v_portion_length, 1, MPI_INT, t.cluster2[i], TAG_LENGTH_WORKER, MPI_COMM_WORLD);
                MPI_Send(v_portion + start_index, v_portion_length, MPI_INT, t.cluster2[i], TAG_V_PORTION_WORKER, MPI_COMM_WORLD);
                cout << "M(2," << t.cluster2[i] << ")\nM(2," << t.cluster2[i] << ")\nM(2," << t.cluster2[i] << ")\n";

                start_index += v_portion_length;
            }

            // receive from the corresponding workers the portion from the array multiplied by 2
            for (int i = 0; i < t.size2; ++i) {
                MPI_Recv(&start_index, 1, MPI_INT, MPI_ANY_SOURCE, TAG_START_INDEX_BACK, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Recv(v_portion + start_index, v_portion_length, MPI_INT, MPI_ANY_SOURCE, start_index, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            }

            // reset v_portion_length and start_index for the current cluster
            v_portion_length = v_portion_length_copy;
            start_index = start_index_copy;

            // send to coordinator 0 start_index, v_portion_length and v_portion for coordinator 2
            MPI_Send(&start_index, 1, MPI_INT, 0, TAG_START_INDEX_BACK_COORD_2, MPI_COMM_WORLD);
            MPI_Send(&v_portion_length, 1, MPI_INT, 0, TAG_LENGTH_BACK_COORD_2, MPI_COMM_WORLD);
            v_portion = (int *)realloc(v_portion, v_portion_length * sizeof(int));
            MPI_Send(v_portion, v_portion_length, MPI_INT, 0, TAG_PORTIONS_BACK_COORD_2, MPI_COMM_WORLD);
            cout << "M(2,0)\nM(2,0)\nM(2,0)\n";

            // receive start_index, v_portion_length and v_portion for coordinator 1, then send the, to coordinator 0
            MPI_Recv(&start_index, 1, MPI_INT, 1, TAG_START_INDEX_BACK_COORD_1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&v_portion_length, 1, MPI_INT, 1, TAG_LENGTH_BACK_COORD_1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            v_portion = (int *)realloc(v_portion, v_portion_length * sizeof(int));
            MPI_Recv(v_portion, v_portion_length, MPI_INT, 1, TAG_PORTIONS_BACK_COORD_1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            MPI_Send(&start_index, 1, MPI_INT, 0, TAG_START_INDEX_BACK_COORD_1, MPI_COMM_WORLD);
            MPI_Send(&v_portion_length, 1, MPI_INT, 0, TAG_LENGTH_BACK_COORD_1, MPI_COMM_WORLD);
            v_portion = (int *)realloc(v_portion, v_portion_length * sizeof(int));
            MPI_Send(v_portion, v_portion_length, MPI_INT, 0, TAG_PORTIONS_BACK_COORD_1, MPI_COMM_WORLD);
            cout << "M(2,0)\nM(2,0)\nM(2,0)\n";
            break;

        }

        default:
            // receive the topology from the corresponding coordinator
            MPI_Recv(&topology_len, 1, MPI_INT, MPI_ANY_SOURCE, TAG_TOPOLOGY_LEN, MPI_COMM_WORLD, &status);

            // save the rank of the coordinator
            int coordinator = status.MPI_SOURCE;

            // allocate memory for the topology string, receive it from the corresponding coordinator, then print it
            topology_str = (char *)calloc(topology_len + 1, sizeof(char));
            MPI_Recv(topology_str, topology_len, MPI_CHAR, coordinator, TAG_TOPOLOGY_STR, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            cout << rank << " -> " << topology_str << endl;

            // receive start_index, v_portion_length and v_portion from the corresponding coordinator
            MPI_Recv(&start_index, 1, MPI_INT, coordinator, TAG_START_INDEX_WORKER, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&v_portion_length, 1, MPI_INT, coordinator, TAG_LENGTH_WORKER, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            v_portion = (int *)calloc(v_portion_length, sizeof(int));
            MPI_Recv(v_portion, v_portion_length, MPI_INT, coordinator, TAG_V_PORTION_WORKER, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // multiply by 2 the v_portion
            for (int i = 0; i < v_portion_length; ++i) {
                v_portion[i] *= 2;
            }

            // send the array already multiplied back to the coordinator
            MPI_Send(&start_index, 1, MPI_INT, coordinator, TAG_START_INDEX_BACK, MPI_COMM_WORLD);
            MPI_Send(v_portion, v_portion_length, MPI_INT, coordinator, start_index, MPI_COMM_WORLD);
            cout << "M(" << rank << "," << coordinator << ")\nM(" << rank << "," << coordinator << ")\n";
    }

    // wait for the processes to be over, then print the result using coordinator 0
    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0) {
        cout << "Rezultat: ";
        for (int i = 0; i < initial_dim; ++i) {
            cout << v[i] << " ";
        }
        cout << endl;
    }

    MPI_Finalize();
}