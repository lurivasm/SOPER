/**
*@brief Ejercicio6
*@author Lucia Rivas Molina <lucia.rivasmolina@estudiante.uam.es>
*@author Daniel Santo-Tomas Lopez <daniel.santo-tomas@estudiante.uam.es>
*@file ejercicio6.c
*@date 2018/03/07
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

/**
*Programa en el que comprobamos que el padre no puede acceder al contenido del hijo
*@brief main del ejercicio6
*/
int main(void){
  char* s = (char*)malloc(sizeof(char)*81);
  if(!s){
    printf("Error al reservar memoria\n");
    exit(EXIT_FAILURE);
  }

  if(fork()){
    printf("Padre esperando\n\n");
    wait(NULL);
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
