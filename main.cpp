#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <math.h>
#include <random>
#define SEED 35791246

#define CHCEWEJSC 1

#define WEJSCIE 1

int losuj(){
  int wycieczka = rand()%100;
  if (wycieczka > 10){
    wycieczka = 0;
  }
  return wycieczka;
}

MPI_Status status;

int main(int argc, char* argv[]) {

    pakiet_t rec_pkt;
    int status;

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
    srand(time(0));
    while(1) {
        int wycieczka = losuj();
        if (wycieczka > 0)
        {
          //rób wszystko
          //wyślij wszystkim "chce wejsc"
          for (int i = 0; i < size; i++)
          {
          		packet_t pkt;
              pkt.info = CHCEWEJSC;
              pkt.timestamp = 0;  //TODO
          		MPI_Send( &pkt, 1, MPI_PAKIET_T, i, WEJSCIE, MPI_COMM_WORLD );
          }
          //czekaj na odpowiedz
          while( !end )
          {
              for (int i = 0; i < size; i++)
              {
                  MPI_Recv( &rec_pkt, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

              }
          }
        }
        else
        {

        }
}
