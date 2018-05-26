#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <math.h>
#include <random>
#include <vector>
#include <unistd.h>

#define SEED 35791246
#define OK 0
#define CHCEWEJSC 1
#define WEJSCIE 1

using namespace std;

typedef struct {
    int info; 
    int timestamp;
} packet_t;

void check_thread_support(int provided)
{
    printf("THREAD SUPPORT: %d\n", provided);
    switch (provided) {
        case MPI_THREAD_SINGLE: 
            printf("Brak wsparcia dla wątków, kończę\n");
            /* Nie ma co, trzeba wychodzić */
	    fprintf(stderr, "Brak wystarczającego wsparcia dla wątków - wychodzę!\n");
	    MPI_Finalize();
	    exit(-1);
	    break;
        case MPI_THREAD_FUNNELED: 
            printf("tylko te wątki, ktore wykonaly mpi_init_thread mogą wykonać wołania do biblioteki mpi\n");
	    break;
        case MPI_THREAD_SERIALIZED: 
            /* Potrzebne zamki wokół wywołań biblioteki MPI */
            printf("tylko jeden watek naraz może wykonać wołania do biblioteki MPI\n");
	    break;
        case MPI_THREAD_MULTIPLE: printf("Pełne wsparcie dla wątków\n");
	    break;
        default: printf("Nikt nic nie wie\n");
    }
}

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
    packet_t *rec_pkt;   //bylo pakiet_t ale zmienilem na packet_t bo chyba bylo zle
    MPI_Status status;

    MPI_Init(&argc, &argv);
    int size,rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    // int provided;
    // MPI_Init_thread(&argc, &argv,MPI_THREAD_MULTIPLE, &provided);
    // check_thread_support(provided);
    // MPI_Init(&argc, &argv);                 //Start MPI
    // MPI_Comm_rank(MPI_COMM_WORLD, &myid);           //get rank of node's process
    // MPI_Comm_size(MPI_COMM_WORLD, &nodenum);
    printf("-1");
    const int nitems=2;
    int blocklengths[2] = {1,1};
    MPI_Datatype typy[2] = {MPI_INT, MPI_INT};
    MPI_Aint offsets[2];



    printf("0");
    offsets[0] = offsetof(packet_t, info);
    offsets[1] = offsetof(packet_t, timestamp);
    printf("1");
    MPI_Type_create_struct(nitems, blocklengths, offsets, typy, &MPI_PAKIET_T);
    MPI_Type_commit(&MPI_PAKIET_T);
    return 0;
    // MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    // MPI_Comm_size(MPI_COMM_WORLD, &size);
    printf("2");
    // int kolejka_procesow[size];
    // int czy_odp[size];
    // print()

    vector<int> kolejka_procesow;
    vector<int> czy_odp;
        printf("3");
    for(int i = 0; i < size; i++)
    {
        kolejka_procesow.push_back(-1);
        czy_odp.push_back(0);
    }
    srand(time(0));
        printf("4");
    while(1) {
        int wycieczka = losuj();
        printf("[%d] [L:%d] Czy mam wycieczkę: %d", rank, zegarLamporta, wycieczka);
        //jesli przyszla wycieczka rob wszystko - wyslij CHCEWEJSC i czekaj na odpowiedzi od innych
        wycieczka = rank % 2;
        if (wycieczka > 0)
        {
            //wyślij wszystkim CHCEWEJSC
            for (int i = 0; i < size; i++)
            {
            	packet_t pkt;
                pkt.info = CHCEWEJSC;
                pkt.timestamp = zegarLamporta;  //wysylamy nasz zegarLamporta
                printf("[%d] [L:%d] Wysyałam wiadomość: CHCEWEJSC", rank, zegarLamporta);
            	MPI_Send(&pkt, 1, MPI_PAKIET_T, i, WEJSCIE, MPI_COMM_WORLD );
            }
            //inkrementuj zegarLamporta po broadcascie
            zegarLamporta++;
            //"odpowiedz" sam do siebie
            czy_odp.at(rank) = 1;
            kolejka_procesow.at(rank) = zegarLamporta;

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
                    printf("[%d] [L:%d] Otrzymałem wiadomość", rank, zegarLamporta);
                    //oznacz w tablicy czy_odp ze juz przyszla odpowiedz od tego procesu
                    czy_odp.at(status.MPI_SOURCE) = 1;
                    //jesli ok to nie ma problemu
                    if(rec_pkt->info == OK)
                    {
                        kolejka_procesow.at(status.MPI_SOURCE) = 0;
                    }
                    //jesli tez CHCEWEJSC przechowujemy jego timestamp
                    else if(rec_pkt->info == CHCEWEJSC)
                    {
                        kolejka_procesow.at(status.MPI_SOURCE) = rec_pkt->timestamp;
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