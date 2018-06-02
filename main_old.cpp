#include <stdio.h>
 #include <stdlib.h>
 #include <stdint.h>
 #include "mpi.h"
 #include <math.h>
 #include <random>
 #include <vector>
 #include <map>
 #include <unistd.h>
 #include <thread>
 #include <algorithm>
 
 #define SEED 35791246
 #define OK 0
 #define CHCEWEJSC 1
 #define WEJSCIE 1
 #define WYCIECZKA 2
 #define SKONCZYLEM 3
 
 using namespace std;
 
 typedef struct {
     int info;
     int timestamp;
     int ludzie;
 } packet_t;
 
 
 int losuj(){
     int wycieczka = rand() % 5;
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
 
 void znajdz_wycieczke(int* wyc_a, int rank, MPI_Datatype MPI_PAKIET_T) {
     while(1) {
         if(*wyc_a == 0){
             int loc = losuj();
             wyc_a = &loc;
             printf("[%d] Wylosowałem %d\n",rank, *wyc_a);
             if(*wyc_a == 1){
 				*wyc_a = 0;
                 packet_t wyceczka_pkt;
                 //Wyślij informację samemu sobie, że otrzymałeś wycieczkę
                 wyceczka_pkt.info = WYCIECZKA;
                 wyceczka_pkt.timestamp = -1;
                 wyceczka_pkt.ludzie = rand() % 10 + 1;
                 MPI_Send(&wyceczka_pkt, 1, MPI_PAKIET_T, rank, WYCIECZKA, MPI_COMM_WORLD );
 
             }
             usleep(2000000);
         }
     }
 }
 
 
 vector<int> sortowanie3(vector<int> kolejka)
 {
 	vector<int> result;
 	vector<int> tmpvec = kolejka;
 	sort(tmpvec.begin(), tmpvec.end());
 
 	for(int i = 0; i < tmpvec.size(); i++)
 	{
 		if(i > 0 && tmpvec[i-1] == tmpvec[i])
 		{
 			continue;
 		}
 		for(int j = 0; j < kolejka.size(); j++)
 		{
 			if(kolejka[j] == tmpvec[i])
 			{
 				result.push_back(j);
 			}
 		}
 	}
 	return result;
 }
 
 
 MPI_Datatype MPI_PAKIET_T;
 
 int main(int argc, char* argv[]) {
     int zegarLamporta = 0;
 
-    int rozmiar_podprzestrzeni = 20;
+    int rozmiar_podprzestrzeni = 12;
     int ludzie_w_podprzestrzeni = 0;
     packet_t *rec_pkt;   //bylo pakiet_t ale zmienilem na packet_t bo chyba bylo zle
     MPI_Status status;
 
     MPI_Init(&argc, &argv);
     int size,rank;
     MPI_Comm_rank(MPI_COMM_WORLD, &rank);
     MPI_Comm_size(MPI_COMM_WORLD, &size);
 
     const int nitems=3;
     int blocklengths[3] = {1,1,1};
     MPI_Datatype typy[3] = {MPI_INT, MPI_INT, MPI_INT};
     MPI_Aint offsets[3];
 
     offsets[0] = offsetof(packet_t, info);
     offsets[1] = offsetof(packet_t, timestamp);
     offsets[2] = offsetof(packet_t, ludzie);
     MPI_Type_create_struct(nitems, blocklengths, offsets, typy, &MPI_PAKIET_T);
     MPI_Type_commit(&MPI_PAKIET_T);
     int wycieczka = 0;
     vector<int> kolejka_procesow;
     vector<int> czy_odp;
     vector<int> liczba_ludzi;
     for(int i = 0; i < size; i++)
     {
         kolejka_procesow.push_back(-1);
         czy_odp.push_back(0);
         liczba_ludzi.push_back(0);
     }
     srand(time(0) + rank);
     printf("Starting thread!\n");
     thread losowanie(znajdz_wycieczke, &wycieczka, rank, MPI_PAKIET_T);
     printf("Thread started!\n");
     while(1) {
         //jesli przyszla wycieczka rob wszystko - wyslij CHCEWEJSC i czekaj na odpowiedzi od innych
         printf("[%d] [L:%d] Czy mam wycieczkę: %d\n", rank, zegarLamporta, wycieczka);
         if (wycieczka > 0)
         {
 
             //wyślij wszystkim CHCEWEJSC
             for (int i = 0; i < size; i++)
             {
                 if (i != rank) {
                     packet_t pkt;
                     pkt.info = CHCEWEJSC;
                     pkt.timestamp = zegarLamporta;  //wysylamy nasz zegarLamporta
                     pkt.ludzie = liczba_ludzi[rank];
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
                     printf("[%d] [L:%d] Otrzymałem wiadomość (czekam na wejscie do sekcji) [%d]\n", rank, zegarLamporta, status.MPI_SOURCE);
                     liczba_ludzi[status.MPI_SOURCE] = allow_packet.ludzie;
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
                     int flag_odp = 1;
                     for(int j = 0; j < size; j++)
                     {
                         if(czy_odp[j] == 0 )
                         {
                             flag_odp = 0;
                             break;
                         }
                     }
 
 
                     //przetwarzanie kiedy dostaniemy odpowiedz od wszystkich
                     if(flag_odp == 1){
                         end = true;
                         //Zerujemy tablice czy inni nam odpowiedzieli
                         for(int j = 0; j < size; j++){
                             czy_odp[j] = 0;
                         }
                         vector<int> posortowane = sortowanie3(kolejka_procesow);
                         bool end1 = false;
                         while( !end1 ){
 							for(int i = 0; i < posortowane.size(); i++){
 								if(kolejka_procesow[posortowane[i]] == -1) continue;
 								//Sprwadzamy czy jest miejsce w podprzestrzeni
 								printf("[%d] [L:%d] Proces [%d] jest %d w kolejce (%d osob)\n", rank, zegarLamporta, posortowane[i], i, liczba_ludzi[posortowane[i]]);
 								if(ludzie_w_podprzestrzeni + liczba_ludzi[posortowane[i]] <= rozmiar_podprzestrzeni){
 									ludzie_w_podprzestrzeni += liczba_ludzi[posortowane[i]];
 									//Jeśli jest nasza kolej to wysyłamy
 									if(posortowane[i] == rank){
 										printf("[%d] [L:%d] Przesyłam wycieczkę (%d osób)! \n", rank, zegarLamporta, liczba_ludzi[rank]);
 										usleep(5000000); //Czas trwania podróży
 										ludzie_w_podprzestrzeni -= liczba_ludzi[posortowane[i]];
 										//Wysyłamy innym, że skończyliśmy
 										for (int i = 0; i < size; i++){
 											if (i != rank) {
 												packet_t pkt;
 												pkt.info = SKONCZYLEM;
 												pkt.timestamp = zegarLamporta;  //wysylamy nasz zegarLamporta
 												pkt.ludzie = liczba_ludzi[rank];
 												printf("[%d] [L:%d] Wysyałam wiadomość: SKONCZYLEM do %d\n", rank, zegarLamporta, i);
 												MPI_Send(&pkt, 1, MPI_PAKIET_T, i, WEJSCIE, MPI_COMM_WORLD);
 												end1 = true;
 											}
 										}
 										liczba_ludzi[rank] = 0;
 										kolejka_procesow.at(rank) = -1;
 										wycieczka = 0;
 									}
 								} else {
 									break;
 								}
 							}
 							if( !end1 )
 							{
 								packet_t test;
 								MPI_Recv(&test, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
 								printf("[%d] [L:%d] Otrzymałem wiadomość od [%d] [L:%d]\n", rank, zegarLamporta, status.MPI_SOURCE, test.timestamp);
 								//aktualizuj zegarLamporta po Recv
 								zegarLamporta = max(zegarLamporta, test.timestamp) + 1;
 								if(test.info == SKONCZYLEM)
 								{
 									printf("[%d] [L:%d] Otrzymałem wiadomość, że proces [%d] skonczyl wycieczke (%d ludzi)\n", rank, zegarLamporta, status.MPI_SOURCE, test.ludzie);
 									ludzie_w_podprzestrzeni -= test.ludzie;
 									liczba_ludzi[status.MPI_SOURCE] = 0;
 									kolejka_procesow.at(status.MPI_SOURCE) = -1;
 									break;
 								}
 							}
                         }
 
 
                         //Wysyłam do wszytkich, że zakończyłem wycieczke (Zwalniam zasoby)
 
                         zegarLamporta++;
                     }
                 //}
             }
         }
         //jesli nie przyszla wycieczka odsylaj innym odpowiedzi
         else
         {
 
             printf("[%d] [L:%d] Oczekuję na wiadomości\n", rank, zegarLamporta);
             for (int i = 0; i < size; i++)
             {
                 packet_t test;
                 MPI_Recv(&test, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                 printf("[%d] [L:%d] Otrzymałem wiadomość od [%d] [L:%d]\n", rank, zegarLamporta, status.MPI_SOURCE, test.timestamp);
                 //aktualizuj zegarLamporta po Recv
                 zegarLamporta = max(zegarLamporta, test.timestamp) + 1;
                 //jesli otrzymano CHCEWEJSC odeslij OK
                 if(test.info == CHCEWEJSC)
                 {
                     kolejka_procesow.at(status.MPI_SOURCE) = test.timestamp;
                     //Sprawdz kto ma pierwszenstwo
                     packet_t pkt;
                     pkt.info = OK;
                     pkt.timestamp = zegarLamporta;
                     pkt.ludzie = 15;
                     printf("[%d] [L:%d] Odpowiadam na żądanie wejścia\n", rank, zegarLamporta);
                     MPI_Send(&pkt, 1, MPI_PAKIET_T, status.MPI_SOURCE, WEJSCIE, MPI_COMM_WORLD );
                     //inkrementuj zegarLamporta po Send
                     zegarLamporta++;
                 }
                 if(test.info == WYCIECZKA)
                 {
                     printf("[%d] [L:%d] Otrzymałem wycieczkę (%d osób)\n", rank, zegarLamporta, test.ludzie);
                     wycieczka = 1;
                     liczba_ludzi[rank] = test.ludzie;
                     break;
                 }
                 if(test.info == SKONCZYLEM)
                 {
                     printf("[%d] [L:%d] Otrzymałem wiadomość, że proces [%d] skonczyl wycieczke (%d ludzi)\n", rank, zegarLamporta, status.MPI_SOURCE, test.ludzie);
                     ludzie_w_podprzestrzeni -= test.ludzie;
                     liczba_ludzi[status.MPI_SOURCE] = 0;
                     kolejka_procesow.at(status.MPI_SOURCE) = -1;
                     break;
                 }
             }
         }
     }
     MPI_Finalize();
 }