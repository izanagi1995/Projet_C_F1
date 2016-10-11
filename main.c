#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <sys/select.h>
#include "defs.h"
#include "manager.h"
#include "car.h"

//DEBUG
#define PRINT_OPAQUE_STRUCT(p)  print_mem((p), sizeof(*(p)))
//END DEBUG


int main(){
   printf("Bienvenue au grand Tournoi de F1!\r\n");
   printf("Nous preparons le tournoi! \r\n");
   int mem_ID;
   void* memoire;
   int pipes[22][2]; //Ce sont les receivers pour les voitures. Seul main y écrit
   int mainPipe[2];  //C'est le receiver pour le main. Les voitures y envoient les events
   int carNums[22] = {44, 6, 5, 7, 3, 33, 19, 77, 11, 27, 26, 55, 14, 22, 9, 12, 20, 30, 8, 21, 31, 94};
   

   if((mem_ID = shmget(MEM_KEY, N_CARS*sizeof(f1Car), IPC_CREAT | 0660 )) < 0){
       perror("Une erreur est survenue durant l'alignement des voitures: ");
       exit(1);
   }

   for(int j = 0; j < N_CARS; j++){
      if((pipe(pipes[j])) != 0){
          perror("Une erreur est survenue lors de l'envoi des invitations: ");
          exit(1);
      }
   }
   if((pipe(mainPipe)) != 0){
       perror("Un hamster a retourné notre serveur mail, nous ne pouvons plus communiquer: ");
       exit(1);
   }

   if ((memoire = shmat(mem_ID, NULL, 0)) == (void*) -1){
       perror("Une erreur est survenue lors de l'assignation des places: ");
       exit(1);
   }

   
    
   //TOUT LE STUFF EST A FAIRE ICI ! :D
   
   f1Car* voitures = (f1Car *)memoire;
   
   for(int i = 0; i < N_CARS; i++){
       voitures[i].writingLock = 1;
       voitures[i].carNumber = carNums[i];
       voitures[i].writingLock = 0;
   }
   
   int pid;
   int nProc;
   for(nProc = 0; nProc < N_CARS; nProc++){
       pid = fork();
       if(pid == 0){
           break;
       }
   }

   if(pid == 0){
        int memSlot = 0;
       
        f1Car* mem = (f1Car *)memoire;
        while(memSlot < N_CARS && mem[memSlot].writingLock == 1){
            memSlot++;
        }
        mem[memSlot].writingLock = 1;
        printf("%d\n", memSlot);
        
        close(pipes[memSlot][1]);
        close(mainPipe[0]);
        int writeTo = mainPipe[1];
        int readFrom = pipes[memSlot][0];      
        
        if(memSlot == 21){
            //Je sais que je suis le dernier, je vais donc prévenir tout le monde
            while(memSlot > 0){
                mem[memSlot--].writingLock = 0;
            }  
            f1CarEvent* readyEvent = malloc(sizeof(*readyEvent));
            readyEvent->carCode = 21;
            readyEvent->eventCode = 0;
            printf("writeTo = %d\r\n", writeTo);
            write(writeTo, readyEvent, sizeof(*readyEvent));
            printf("write done\r\n");
        }
        initCar(mem[memSlot], (void *)mem, memSlot, readFrom, writeTo);
        startCar();
    }else{
        //close(pipes[memSlot][1]);
        close(mainPipe[1]);
        int readFrom = mainPipe[0];
        int writeTo[22];
        for(int d = 0; d < N_CARS; d++){
            close(pipes[d][0]);
            writeTo[d] = pipes[d][1];
        }
        printf("Pipe OK");
        initReadStream(readFrom, writeTo);
        initMemory(memoire);
        printf("READY\r\n");
        startManager();
   }
   shmdt(memoire);
   shmctl(mem_ID, IPC_RMID, NULL);
}
