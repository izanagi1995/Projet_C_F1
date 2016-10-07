#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "defs.h"




int main(int argc, char *argv[]){
   printf("Bienvenue au grand Tournoi de F1!\r\n");
   printf("Nous preparons le tournoi! \r\n");
   int mem_ID;
   void* memoire;
   int pipes[22][2]; //Ce sont les receivers pour les voitures. Seul main y écrit
   int mainPipe[2];  //C'est le receiver pour le main. Les voitures y envoient les events
   int carNums[22] = {44, 6, 5, 7, 3, 33, 19, 77, 11, 27, 26, 55, 14, 22, 9, 12, 20, 30, 8, 21, 31, 94};
   
   int mustDie = 0;

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
   
   *((f1Car*)memoire) = *voitures;
   f1Car* nouvellesVoitures = (f1Car *)memoire;
   f1Car laPremiereVoiture2 = voitures[0];
   printf("%d\n", laPremiereVoiture2.carNumber);
   
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
        int writingPipe = 0;
       
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
            f1CarEvent readyEvent;
            readyEvent.carCode = 21;
            readyEvent.eventCode = 0;
            write(writeTo, &readyEvent, sizeof(f1CarEvent));
        }
    }else{
        //close(pipes[memSlot][1]);
        close(mainPipe[1]);
        int readFrom = mainPipe[0];
        int writeTo[22];
        for(int d = 0; d < N_CARS; d++){
            close(pipes[d][0]);
            writeTo[d] = pipes[d][1];
        }
        while(!mustDie){
            //Read pipe loop
            //GUI
            //Response
            //A un moment ou un autre...
            mustDie = 1;
        }                


        //Je suis ton pere!
//           _________
//           III| |III
//         IIIII| |IIIII
//        IIIIII| |IIIIII
//        IIIIII| |IIIIII
//       IIIIIII| |IIIIIII
//       IIIIIII\ /IIIIIII
//      II (___)| |(___) II
//     II  \    /D\    /  II
//    II  \ \  /| |\  / /  II
//   II    \_\/|| ||\/_/    II
//  II     / O-------O \     II
// II_____/   \|||||/   \_____II
//       /               \
//       |_______________|
   }
    
   shmdt(memoire);
   shmctl(mem_ID, IPC_RMID, NULL);
}
