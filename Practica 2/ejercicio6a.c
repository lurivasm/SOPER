/**
*@brief Ejercicio6a
*@author Lucia Rivas Molina
*@author Daniel Santo-Tomas Lopez
*@date 31/03/2018
*@file ejercicio6a.c  
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

/**
*@brief Numeros que cuenta el proceso
*/
#define NUM_PROC 5
/**
*@brief Segundos que espera a la alarma
*/
#define SEC 40

/**
*@brief Main del ejercicio 6a
*@return EXIT_SUCCESS o EXIT_FAILURE
*/
int main (void) {
  int pid, counter, error;
  sigset_t set, oset;

  /*Metemos las señales en el set para bloquearlas ya que el hijo las hereda*/
  sigemptyset(&set);
  sigaddset(&set, SIGUSR1);
  sigaddset(&set, SIGUSR2);
  sigaddset(&set, SIGALRM);

  /*Realizamos el fork*/
  pid = fork();

  /*En caso de error*/
  if(pid < 0){
    printf("ERROR\n");
    exit(EXIT_FAILURE);
  }

  /*En el hijo*/
  else if (pid == 0){
    alarm(SEC);
    while(1){
      /*Bloqueamos las tres señales*/
      /*Si SIGALRM y SIGUSR1 no pertenecen a la mascara las agregamos*/
      if(sigismember(&set, SIGALRM) == 0 && sigismember(&set, SIGUSR1) == 0){
        sigaddset(&set, SIGUSR1);
        sigaddset(&set, SIGALRM);
      }
      /*Hacemos un set a la mascara para aplicar las funciones añadidas*/
      error = sigprocmask(SIG_SETMASK, &set,&oset);
      if(error){
        printf("ERROR\n");
        exit(EXIT_FAILURE);
      }
      /*Realizamos la cuenta*/
      for (counter = 0; counter < NUM_PROC; counter++){
        printf("%d\n", counter);
        sleep(1);
      }
      /*Desbloqueamos dos señales*/
      sigdelset(&set, SIGUSR1);
      sigdelset(&set, SIGALRM);
      error = sigprocmask(SIG_SETMASK, &set,&oset);
      if(error){
        printf("ERROR\n");
        exit(EXIT_FAILURE);
      }
    }
  }
  while(wait(NULL)>0);
  exit(EXIT_SUCCESS);
}
