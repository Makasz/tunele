#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <math.h>
#include <random>
#define SEED 35791246

#define OK 0
#define CHCEWEJSC 1

#define WEJSCIE 1

typedef struct {
    int appdata;
    int info; 
    int timestamp;
    int src; 
} packet_t;

int losuj(){
    int wycieczka = rand()%100;
    if (wycieczka > 10)
    {
        wycieczka = 0;
    }
    return wycieczka;
}

int max(int a, int b)
{
    if (a>b)  return a;
    else  return b;
}

MPI_Datatype MPI_PAKIET_T;

int main(int argc, char* argv[]) {
    int zegarLamporta = 0;
    int myid, nodenum, rank, size;
    packet_t *rec_pkt;   //bylo pakiet_t ale zmienilem na packet_t bo chyba bylo zle
    MPI_Status status;

    MPI_Init(&argc, &argv);                 //Start MPI
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);           //get rank of node's process
    MPI_Comm_size(MPI_COMM_WORLD, &nodenum);

    const int nitems=2;
    int blocklengths[2] = {1,1};
    MPI_Datatype typy[2] = {MPI_INT, MPI_INT};
    MPI_Aint offsets[2];

    offsets[0] = offsetof(packet_t, info);
    offsets[1] = offsetof(packet_t, timestamp);

    MPI_Type_create_struct(nitems, blocklengths, offsets, typy, &MPI_PAKIET_T);
    MPI_Type_commit(&MPI_PAKIET_T);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int kolejka_procesow[size];
    int czy_odp[size];
    for(int i = 0; i < size; i++)
    {
        kolejka_procesow[i] = -1;
        czy_odp[i] = 0;
    }
    srand(time(0));
    while(1) {
        int wycieczka = losuj();
        //jesli przyszla wycieczka rob wszystko - wyslij CHCEWEJSC i czekaj na odpowiedzi od innych
        if (wycieczka > 0)
        {
            //wyślij wszystkim CHCEWEJSC
            for (int i = 0; i < size; i++)
            {
            	packet_t pkt;
                pkt.info = CHCEWEJSC;
                pkt.timestamp = zegarLamporta;  //wysylamy nasz zegarLamporta
            	MPI_Send(&pkt, 1, MPI_PAKIET_T, i, WEJSCIE, MPI_COMM_WORLD );
            }
            //inkrementuj zegarLamporta po broadcascie
            zegarLamporta++;
            //"odpowiedz" sam do siebie
            czy_odp[rank] = 1;
            kolejka_procesow[rank] = zegarLamporta;

            //czekaj na odpowiedz od wszystkich
            bool end = false;
            while( !end )
            {
                //for (int i = 0; i < size; i++)  //ten for chyba nie jest potrzebny
                //{
                    //odbierz pakiet od dowolnego procesu
                    MPI_Recv( &rec_pkt, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                    //aktualizuj zegarLamporta po Recv
                    zegarLamporta = max(zegarLamporta, rec_pkt->timestamp) + 1;
                    //oznacz w tablicy czy_odp ze juz przyszla odpowiedz od tego procesu
                    czy_odp[status.MPI_SOURCE] = 1;
                    //jesli ok to nie ma problemu
                    if(rec_pkt->info == OK)
                    {
                        kolejka_procesow[status.MPI_SOURCE] = 0;
                    }
                    //jesli tez CHCEWEJSC przechowujemy jego timestamp
                    else if(rec_pkt->info == CHCEWEJSC)
                    {
                        kolejka_procesow[status.MPI_SOURCE] = rec_pkt->timestamp;
                    }
                    //sprawdzamy czy mamy juz odpowiedz od wszystkich
                    for(int j = 0; j < size; j++)
                    {
                        if(czy_odp[j] == 0)
                        {
                            break;
                        }
                        end = true;
                    }
                //}
            }
        }
        //jesli nie przyszla wycieczka odsylaj innym odpowiedzi
        else
        {
            for (int i = 0; i < size; i++)
            {
                MPI_Recv( &rec_pkt, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                //aktualizuj zegarLamporta po Recv
                zegarLamporta = max(zegarLamporta, rec_pkt->timestamp) + 1;
                //jesli otrzymano CHCEWEJSC odeslij OK
                if(rec_pkt->info == CHCEWEJSC)
                {
                    packet_t pkt;
                    pkt.info = OK;
                    pkt.timestamp = zegarLamporta;
                    MPI_Send(&pkt, 1, MPI_PAKIET_T, status.MPI_SOURCE, WEJSCIE, MPI_COMM_WORLD );
                    //inkrementuj zegarLamporta po Send
                    zegarLamporta++;
                }
            }
        }
    }
}