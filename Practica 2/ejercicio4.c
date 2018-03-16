#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>


void main(){
int padre, hijo1, hijo2, i, N, status;

hijo1 = fork();
printf("%d",hijo1);
while(1){

  if(hijo1 != 0 ){
    pause();
    if(N < 5){
      if((hijo2 = fork()) == 0){
        kill(hijo1,SIGUSR1);
      }
      waitpid(hijo1, &status, WUNTRACED | WCONTINUED);
      hijo1 = hijo2;
      N++;
    }

    else{
      kill(hijo1,SIGUSR1);
      waitpid(hijo1, &status, WUNTRACED | WCONTINUED);
      break;
    }
  }
  if(hijo1 == 0){
    for(i = 0 ; i < 10 ; i++){
      printf("Soy %d y estoy trabajando",getpid());
      usleep(1000000);
    }
    kill(getppid(),SIGUSR1);
    while(pause()){
      printf("Soy %d y estoy trabajando",getpid());
      usleep(1000000);
    }
    exit(EXIT_SUCCESS);
  }
}
  exit(EXIT_SUCCESS);
}
