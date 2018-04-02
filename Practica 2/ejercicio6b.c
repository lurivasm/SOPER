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
#define NUM_PROC 5


void captura(int sennal)
{
 printf("\nSoy %d y he recibido la señal SIGTERM\n", getpid());
 exit(0);
}


int main (void){
  int pid, counter;
  if (signal(SIGTERM, captura) == SIG_ERR){
    puts("Error en la captura");
    exit (EXIT_FAILURE);
  }
  pid = fork();
  if (pid == 0){
    while(1){
      for (counter = 0; counter < NUM_PROC; counter++){
        printf("%d\n", counter);
        sleep(1);
      }
      sleep(3);
    }
  }
  else{
    sleep(40);
    kill(pid,SIGTERM);
  }
  while(wait(NULL)>0);
  exit(EXIT_SUCCESS);
}
