#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>


void main(){

  int pid,i;

  /*En cada iteracion del bucle se crea un hjo*/
  for(i = 0 ; i < 4 ; i++){
    /*Almacenamos el valor del pid del hijo. En este if entra el padre*/
    if((pid = fork()) != 0){
      usleep(5000000);
      /*Espera 5 segundos y envia la señal al hijo*/
      kill(pid,SIGTERM);
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


}
