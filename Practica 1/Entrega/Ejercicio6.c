#include <stdio.h>
#include <stdlib.h>

/*El padre no puede acceder al contenido del hijo*/
/*Al hacerlo fuera queda liberada la memoria*/
int main(void){
  char* s = (char*)malloc(sizeof(char)*81);
  if(!s){
    printf("Error al reservar memoria\n");
    exit(EXIT_FAILURE);
  }

  int n;


  if(fork()){
    printf("Padre esperando\n\n");
    wait();
    printf("Padre imprime: %s\n",s);
    printf("Padre termina\n\n");
  }

  else{
    printf("Hijo empieza\n");
    printf("Introduce un nombre: ");
    fgets(s, 81, stdin);
    printf("Hijo imprime: %s\n",s);

    printf("Hijo termina\n\n");
  }

  free(s);
  exit(EXIT_SUCCESS);
}
