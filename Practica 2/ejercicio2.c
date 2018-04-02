/**
*@brief Ejercicio2
*@author Lucia Rivas Molina
*@author Daniel Santo-Tomas Lopez
*@date 31/03/2018
*@file ejercicio2.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>


int main(){

  int pid,i;

  /*En cada iteracion del bucle se crea un hjo*/
  for(i = 0 ; i < 4 ; i++){
    /*En caso de error*/
    if((pid = fork()) < 0){
      printf("ERROR\n");
      exit(EXIT_FAILURE);
    }
    /*Almacenamos el valor del pid del hijo. En este if entra el padre*/
    else if(pid > 0){
      usleep(5000000);
      /*Espera 5 segundos y envia la señal al hijo*/
      kill(pid,SIGTERM);
      wait(NULL);
    }
    /*En el eslse entra el hijo*/
    else{
      /*Imprime el mensaje, espera 30 segundos, imprime el segundo mensaje y termina*/
      printf("Soy el proceso hijo %d \n",getpid());
      usleep(30000000);
      printf("Soy el proceso hijo %d y ya me toca terminar",getpid());
      exit(EXIT_SUCCESS);
    }
  }
exit(EXIT_SUCCESS);

}
