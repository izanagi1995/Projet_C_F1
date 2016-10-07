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
   
   int carNums[22] = {44, 6, 5, 7, 3, 33, 19, 77, 11, 27, 26, 55, 14, 22, 9, 12, 20, 30, 8, 21, 31, 94};

   if((mem_ID = shmget(MEM_KEY, N_CARS*sizeof(f1Car), IPC_CREAT | 0660 )) < 0){
       perror("Une erreur est survenue durant l'alignement des voitures: ");
       exit(1);
   }
   if ((memoire = shmat(mem_ID, NULL, 0)) == (void*) -1){
       perror("Une erreur est survenue lors de l'envoi des invitations aux concurrents: ");
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
       f1Car* mem = (f1Car *)memoire;
       while(memSlot < N_CARS && mem[memSlot].writingLock == 1){
           memSlot++;
       }
       mem[memSlot].writingLock = 1;
       printf("%d\n", memSlot);
       if(memSlot == 21){
           //Je sais que je suis le dernier, je vais donc prévenir tout le monde
           while(memSlot > 0){
               mem[memSlot--].writingLock = 0;
           }  
       }
   }else{
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
