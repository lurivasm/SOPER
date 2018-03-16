#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#define TAM 5

void captura(){
  return;
}

int main(){
int  i, j, status;
pid_t hijo[5];

signal(SIGUSR1, captura);

for(i = 0; i < TAM; i++){
  hijo[i] = fork();

  if(hijo[i] < 0){
    printf("ERROR\n");
    exit(EXIT_FAILURE);
  }

  else if(hijo[i] > 0 ){
    /*Si estamos en la primera iteración no esperamos a ningún hijo anterior*/
    if(i != 0){
      waitpid(hijo[i-1], &status, WUNTRACED | WCONTINUED);
    }
    pause();
    continue;
  }

  else if(hijo[i] == 0){
    /*En caso de estar en la primera iteración el hijo no elimina ningún otro proceso hijo*/
    if(i != 0){
      kill(hijo[i-1], SIGTERM);
    }
    for(j = 0 ; j < 10 ; j++){
      printf("Soy %d y estoy trabajando\n",getpid());
      fflush(stdout);
      sleep(1);
    }
    kill(getppid(),SIGUSR1);
    while(1){
      printf("Soy %d y estoy trabajando\n",getpid());
      sleep(1);
    }
    exit(EXIT_SUCCESS);
  }
}
  kill(hijo[i], SIGTERM);
  waitpid(hijo[i], &status, WUNTRACED | WCONTINUED);
  exit(EXIT_SUCCESS);
}
