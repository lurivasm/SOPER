/**
*@brief Ejercicio 12a
*@author Lucia Rivas Molina <lucia.rivasmolina@estudiante.uam.es>
*@author Daniel Santo-Tomas Lopez <daniel.santo-tomas@estudiante.uam.es>
*@file ejercicio12a.c
*@date 2018/03/07
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <math.h>
#include <sys/time.h>

/**
*@brief numero de forks que queremos hacer
*/
#define TAM_FORK 100

/**
*@brief Estructura pedida en el enunciado
*/
typedef struct{
  char cadena[100];
  int num;
} Estructura;


/**
*@brief Calcula los primeros N primos
*@param numero de primos a calcular
*@return void pues solo necesitamos calcular el tiempo
*/
void primerosNPrimos(int N){
  int n = 2, i, es_primo, cont = 0;
  /*int *array = (int*)malloc(sizeof(int)*N); Comentado pues solo es necesario calcular cuanto tarda*/

  if (N <= 0) {
    return;
  }
  while (N > 0) {
        /* determinar si n es primo */
        es_primo = 1;
        for (i = 2; i < sqrt(n); ++i) {
            if (n % i == 0) {
                es_primo = 0;
                break;
            }
        }
        /* Actualizar el array y contador */
        if (es_primo) {
          /*array[cont] = n;*/
          cont++;
          N--;
        }
        n++;
    }
    /*return array;*/
    return;
}

/**
*Programa que calcula cuánto tiempo tarda en calcular los primeros n primos con forks
*@brief Funcion main del ejercicio12a
*@param numero de primos a calcular
*/
int main(int argc, char**argv){

  int i,n;
  double secs = 0;
  struct  timeval t_ini, t_fin;

  /*Reservamos memoria para la estructura pedida*/
  Estructura *e = (Estructura*)malloc(sizeof(Estructura));

  if(argc < 2) {
    printf("Error en los argumentos.Falta n de primos a calcular\n");
    exit(EXIT_FAILURE);
  }

  /*Convertimos los argumentos en int*/
  n = atoi(argv[1]);

  /*Hacemos los forks*/
  gettimeofday(&t_ini, NULL);
  for(i = 0; i < TAM_FORK; i++){
    if(fork()){
      continue;
     }
    else {
      primerosNPrimos(n);
      exit(EXIT_SUCCESS);
    }
  }
  for(i = 0; i < TAM_FORK; i++){
    wait(NULL);
  }

  /*Calculamos el tiempo y liberamos*/
  gettimeofday(&t_fin, NULL);
  secs += (double)(t_fin.tv_sec + (double)t_fin.tv_usec/1000000) - (double)(t_ini.tv_sec + (double)t_ini.tv_usec/1000000);

  printf(" Has tardado %f segundos\n",secs);
  free(e);
  exit(EXIT_SUCCESS);
}
