/**
*@brief Ejercicio4b
*@author Lucia Rivas Molina <lucia.rivasmolina@estudiante.uam.es>
*@author Daniel Santo-Tomas Lopez <daniel.santo-tomas@estudiante.uam.es>
*@file ejercicio4b.c
*@date 2018/03/07
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define NUM_PROC 6

/**
*Ejercicio que realiza 6 forks con un wait
*@brief main del ejercicio4b
*/
int main (void){
   int pid;
   int i;
   for (i=0; i <= NUM_PROC; i++){
     if (i % 2 == 0) {
       if ((pid = fork()) <0 ){
         printf("Error haciendo fork\n");
         exit(EXIT_FAILURE);
       }else if (pid == 0){
         printf("HIJO %d\t PADRE %d\n", getpid(), getppid());
       }else{
         printf ("PADRE %d\n", getpid());
       }
     }
   }
   wait(NULL);
   exit(EXIT_SUCCESS);
}
