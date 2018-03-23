#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

#define NUM_PROC 5
#define SEC 40


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
  if(pid < 0){
    printf("ERROR\n");
    exit(EXIT_FAILURE);
  }
  else if (pid == 0){
    alarm(10);
    while(1){
      /*Bloqueamos las tres señales*/
      if(sigismember(&set, SIGALRM) == 0 && sigismember(&set, SIGUSR1) == 0){
        sigaddset(&set, SIGUSR1);
        sigaddset(&set, SIGALRM);
      }
      error = sigprocmask(SIG_BLOCK, &set,&oset);
      /*if(error){
        printf("ERROR\n");
        exit(EXIT_FAILURE);
      }*/
      for (counter = 0; counter < NUM_PROC; counter++){
        printf("%d\n", counter);
        sleep(1);
      }
      /*Desbloqueamos dos señales*/
      sigdelset(&set, SIGUSR1);
      sigdelset(&set, SIGALRM);
      error = sigprocmask(SIG_BLOCK, &set,&oset);
    }
  }
  while(wait(NULL)>0);
  exit(EXIT_SUCCESS);
}
