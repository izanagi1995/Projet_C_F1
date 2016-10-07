#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "defs.h"

int main(int argc, char *argv[]){
   printf("Bienvenue au grand Tournoi de F1!\r\n");
   printf("Nous preparons le tournoi! \r\n");
   int mem_ID;
   void* memoire;

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
   f1Car laPremiereVoiture = voitures[0];      
   printf("%d\n", laPremiereVoiture.carNumber);   
   voitures[0].carNumber = 33;
   
   *((f1Car*)memoire) = *voitures;
   f1Car* nouvellesVoitures = (f1Car *)memoire;
   f1Car laPremiereVoiture2 = voitures[0];
   printf("%d\n", laPremiereVoiture2.carNumber);
 
   shmdt(memoire);
   shmctl(mem_ID, IPC_RMID, NULL);
   printf("Merci d'avoir participé à notre tournoi! Nous vous disons merci, et à l'an prochain!\r\n");
}
