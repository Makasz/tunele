#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "mpi.h"
#include <math.h>
#include <random>
#include <vector>
#include <unistd.h>
#include <thread>

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
    int wycieczka = rand() % 10;
    if (wycieczka == 1)
    {
        return 1;
    }
    return 0;
}

int max(int a, int b)
{
    if (a>b)  return a;
    else  return b;
}

void znajdz_wycieczke(int* wyc_a) {
    if(*wyc_a == 0){
        int loc = losuj();
        wyc_a = &loc;
        printf("Wylosowałem %d\n", *wyc_a);
        usleep(5000000);
    }
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
    const int nitems=2;
    int blocklengths[2] = {1,1};
    MPI_Datatype typy[2] = {MPI_INT, MPI_INT};
    MPI_Aint offsets[2];

    offsets[0] = offsetof(packet_t, info);
    offsets[1] = offsetof(packet_t, timestamp);
    MPI_Type_create_struct(nitems, blocklengths, offsets, typy, &MPI_PAKIET_T);
    MPI_Type_commit(&MPI_PAKIET_T);

    // MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    // MPI_Comm_size(MPI_COMM_WORLD, &size);
    // int kolejka_procesow[size];
    // int czy_odp[size];
    // print()

    vector<int> kolejka_procesow;
    vector<int> czy_odp;
    for(int i = 0; i < size; i++)
    {
        kolejka_procesow.push_back(-1);
        czy_odp.push_back(0);
    }
    srand(time(0));

    pthread_t thread_id;
    int rc;
    int wycieczka = 0;
    printf("Starting thread!\n");
    thread losowanie(znajdz_wycieczke, &wycieczka);
    printf("Thread started!\n");
    while(1) {
        //jesli przyszla wycieczka rob wszystko - wyslij CHCEWEJSC i czekaj na odpowiedzi od innych
        printf("[%d] [L:%d] Czy mam wycieczkę: %d\n", rank, zegarLamporta, wycieczka);
        if (wycieczka)
        {
            wycieczka = 0;
            //wyślij wszystkim CHCEWEJSC
            for (int i = 0; i < size; i++)
            {
                if (i != rank) {
                    packet_t pkt;
                    pkt.info = CHCEWEJSC;
                    pkt.timestamp = zegarLamporta;  //wysylamy nasz zegarLamporta
                    printf("[%d] [L:%d] Wysyałam wiadomość: CHCEWEJSC do %d\n", rank, zegarLamporta, i);
                    MPI_Send(&pkt, 1, MPI_PAKIET_T, i, WEJSCIE, MPI_COMM_WORLD);
                }
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
                    packet_t allow_packet;
                    printf("[%d] [L:%d] Czekam na wiadomości czy mogę wejść\n", rank, zegarLamporta);
                    MPI_Recv(&allow_packet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                    //aktualizuj zegarLamporta po Recv
                    zegarLamporta = max(zegarLamporta, allow_packet.timestamp) + 1;
                    printf("[%d] [L:%d] Otrzymałem wiadomość żę mogę wejść\n", rank, zegarLamporta);
                    //oznacz w tablicy czy_odp ze juz przyszla odpowiedz od tego procesu
                    czy_odp.at(status.MPI_SOURCE) = 1;
                    //jesli ok to nie ma problemu
                    if(allow_packet.info == OK)
                    {
                        kolejka_procesow.at(status.MPI_SOURCE) = -1;
                    }
                    //jesli tez CHCEWEJSC przechowujemy jego timestamp
                    else if(allow_packet.info == CHCEWEJSC)
                    {
                        kolejka_procesow.at(status.MPI_SOURCE) = allow_packet.timestamp;
                        //Narazie wystarczy, że dostaniemy odpowiedź od każdego żeby wysłać wycieczkę
                    }
                    //sprawdzamy czy mamy juz odpowiedz od wszystkich
                    for(int j = 0; j < size; j++)
                    {
                        if(czy_odp[j] == 0 )
                        {
                            break;
                        }
                        end = true;
                        printf("[%d] [L:%d] Przesyłam wycieczkę! \n", rank, zegarLamporta);
                        usleep(2000000);
                        break;
                    }
                //}
            }
        }
        //jesli nie przyszla wycieczka odsylaj innym odpowiedzi
        else
        {
            
            printf("[%d] [L:%d] Oczekuję na żądania\n", rank, zegarLamporta);
            for (int i = 0; i < size; i++)
            {   
                packet_t test;
                MPI_Recv(&test, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                printf("[%d] [L:%d] Otrzymałem żądanie od [%d] [L:%d]\n", rank, zegarLamporta, status.MPI_SOURCE, test.timestamp);
                //aktualizuj zegarLamporta po Recv
                zegarLamporta = max(zegarLamporta, test.timestamp) + 1;
                //jesli otrzymano CHCEWEJSC odeslij OK
                if(test.info == CHCEWEJSC)
                {
                    packet_t pkt;
                    pkt.info = OK;
                    pkt.timestamp = zegarLamporta;
                    printf("[%d] [L:%d] Odpowiadam na żądanie\n", rank, zegarLamporta);
                    MPI_Send(&pkt, 1, MPI_PAKIET_T, status.MPI_SOURCE, WEJSCIE, MPI_COMM_WORLD );
                    //inkrementuj zegarLamporta po Send
                    zegarLamporta++;
                }
            }
        }
    }
    MPI_Finalize();
}