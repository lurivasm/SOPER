/**
*@brief Ejercicio 4
*@author Lucia Rivas Molina <lucia.rivasmolina@estudiante.uam.es>
*@author Daniel Santo-Tomas Lopez <daniel.santo-tomas@estudiante.uam.es>
*@file ejercicio4.c
*@date 2018/03/07
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_PROC 6

/**
*Ejercicio que realiza 6 forks sin waits
*@brief main del ejercicio 4
*/
int main (void){
   int pid;
   int i;
   for (i = 0; i <= NUM_PROC; i++){
     if (i % 2 == 0) {
       pid = fork();
       if (pid < 0 ){
           printf("Error al emplear fork\n");
           exit(EXIT_FAILURE);
         }
        else if (pid == 0){
           printf("HIJO %d\t PADRE %d\n", getpid(), getppid());
        }
        else{
           printf ("PADRE %d \n", getpid());
        }
     }
   }
   exit(EXIT_SUCCESS);
}
