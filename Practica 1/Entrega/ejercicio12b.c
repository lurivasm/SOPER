/**
*@brief Ejercicio 12b
*@author Lucia Rivas Molina <lucia.rivasmolina@estudiante.uam.es>
*@author Daniel Santo-Tomas Lopez <daniel.santo-tomas@estudiante.uam.es>
*@file ejercicio12B.c
*@date 2018/03/07
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <time.h>
#include <math.h>
#include <sys/time.h>

/**
*@brief numero de hilos que queremos hacer
*/
#define TAM_HILO 100

/**
*Estructura pedida en el enunciado
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
void *primerosNPrimos(int *N){
  int n = 2, i, es_primo, cont = 0;
  /*int *array = (int*)malloc(sizeof(int)*N);*/

  if (*N <= 0) {
    return NULL;
  }
  while (*N > 0) {
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
          (*N)--;
        }
        n++;
    }
    /*return array;*/
    return NULL;
}

/**
*Programa que calcula el tiempo que tarda en calcular los primeros n con hilos
*@brief Funcion main del ejercicio12b
*@param numero de primos a calcular
*/
int main(int argc, char**argv){

  int i,n;
  double secs = 0;
  struct  timeval t_ini, t_fin;
  pthread_t array[TAM_HILO];
  Estructura *e = (Estructura*)malloc(sizeof(Estructura));

  /*En caso de error en los parámetros*/
  if(argc < 2) {
    printf("Error en los argumentos.Falta n de primos a calcular\n");
    exit(EXIT_FAILURE);
  }
  /*Convertimos en int los argumentos*/
  n = atoi(argv[1]);

  /*Creamos los hilos*/
  gettimeofday(&t_ini, NULL);
  for(i = 0; i < TAM_HILO; i++){
    pthread_create(&array[i], NULL, (void * (*)(void *))primerosNPrimos, (void *)&n);
  }
  /*Hacemos un join a los hijos*/
  for(i = 0; i < TAM_HILO; i++){
    pthread_join(array[i], NULL);
  }

  /*Calculamos el tiempo y liberamos*/
  gettimeofday(&t_fin, NULL);
  secs += (double)(t_fin.tv_sec + (double)t_fin.tv_usec/1000000) - (double)(t_ini.tv_sec + (double)t_ini.tv_usec/1000000);

  printf(" Has tardado %f segundos\n",secs);
  free(e);
  exit(EXIT_SUCCESS);
}
