/**
*@brief Ejercicio6b
*@author Lucia Rivas Molina
*@author Daniel Santo-Tomas Lopez
*@date 31/03/2018
*@file ejercicio6b.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

/**
*@brief Contador del hijo
*/
#define NUM_PROC 5

/**
*@brief Funcion captura de la señal
*@param sennal Señal recibida
*/
void captura(int sennal);

/**
*@brief Main del ejercicio 6b
*@return EXIT_SUCCESS o EXIT_FAILURE
*/
int main (void){
  int pid, counter;
  
  /*Capturamos la señal*/
  if (signal(SIGTERM, captura) == SIG_ERR){
    puts("Error en la captura");
    exit (EXIT_FAILURE);
  }
  pid = fork();
  /*En caso de error*/
  if(pid < 0){
    perror("Error en el fork");
    exit(EXIT_FAILURE);
  }

  /*En el hijo contamos y dormimos en bucle hasta recibir la señal del padre*/
  else if (pid == 0){
    while(1){
      for (counter = 0; counter < NUM_PROC; counter++){
        printf("%d\n", counter);
        sleep(1);
      }
      sleep(3);
    }
  }
  
  /*En el padre esperamos 40 segundos y mandamos una señal al hijo*/
  else{
    sleep(40);
    kill(pid,SIGTERM);
  }

  /*Esperamos al hijo y salimos*/
  while(wait(NULL)>0);
  exit(EXIT_SUCCESS);
}

/*Funcion que captura la señal*/
void captura(int sennal)
{
 printf("\nSoy %d y he recibido la señal SIGTERM\n", getpid());
 exit(0);
}
