#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <math.h>
#define SEED 35791246

int main(int argc, char* argv[]) {

     MPI_Init(&argc, &argv);                 //Start MPI
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);           //get rank of node's process
    MPI_Comm_size(MPI_COMM_WORLD, &nodenum);

    const int nitems=2;
    int       blocklengths[2] = {1,1};
    MPI_Datatype typy[2] = {MPI_INT, MPI_INT};
    MPI_Aint     offsets[2];

    offsets[0] = offsetof(packet_t, info);
    offsets[1] = offsetof(packet_t, timestamp);

    MPI_Type_create_struct(nitems, blocklengths, offsets, typy, &MPI_PAKIET_T);
    MPI_Type_commit(&MPI_PAKIET_T);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    while(1) {
        
    }
}