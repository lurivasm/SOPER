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

/**
*@brief numero de HIJOS
*/
#define HIJOS 4
/**
*@brief Main del ejercicio 2
*@return EXIT_FAILURE o EXIT_SUCCESS
*/
int main(){

  int pid, i;

  /*En cada iteracion del bucle se crea un hjo*/
  for(i = 0 ; i < HIJOS; i++){
    /*En caso de error*/
    if((pid = fork()) < 0){
      printf("ERROR\n");
      exit(EXIT_FAILURE);
    }
    /*Almacenamos el valor del pid del hijo. En este if entra el padre*/
    else if(pid > 0){
      sleep(5);
      /*Espera 5 segundos y envia la señal al hijo*/
      kill(pid, SIGTERM);
      wait(NULL);
    }

    /*En el else entra el hijo*/
    else{
      /*Imprime el mensaje, espera 30 segundos, imprime el segundo mensaje y termina*/
      printf("Soy el proceso hijo %d \n",getpid());
      sleep(30);
      printf("Soy el proceso hijo %d y ya me toca terminar",getpid());
      exit(EXIT_SUCCESS);
    }
  }

exit(EXIT_SUCCESS);

}
