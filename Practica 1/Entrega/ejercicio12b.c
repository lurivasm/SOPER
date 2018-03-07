/**
*@brief Ejercicio 12b
*@author Lucia Rivas Molina
*@author Daniel Santo-Tomas Lopez
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

typedef struct{
  char cadena[100];
  int num;
} Estructura;


/**
*@brief Calcula los primeros N primos
*@author Lucia Rivas Molina
*@author Daniel Santo-Tomas Lopez
*@param numero de primos a calcular
*@return void pues solo necesitamos calcular el tiempo
*/
void *primerosNPrimos(int *N){
  int n = 2, i, es_primo, cont = 0;
  /*int *array = (int*)malloc(sizeof(int)*N);*/

  if (*N <= 0) return;
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
          *N--;
        }
        n++;
    }
    /*return array;*/
}

/**
*@brief Funcion main
*@author Lucia Rivas Molina
*@author Daniel Santo-Tomas Lopez
*@param numero de primos a calcular
*/
void main(int argc, char**argv){

  int i,n;
  double secs = 0;
  struct  timeval t_ini, t_fin;
  pthread_t array[100];
  Estructura *e = (Estructura*)malloc(sizeof(Estructura));

  if(argc < 2) {
    printf("Error en los argumentos.Falta n de primos a calcular\n");
    return ;
  }

  n = atoi(argv[1]);

  gettimeofday(&t_ini, NULL);
  for(i = 0; i < 100; i++){
    pthread_create(&array[i], NULL, primerosNPrimos, (void *)&n);
  }
  for(i = 0; i < 100; i++){
    pthread_join(array[i], NULL);
  }

  gettimeofday(&t_fin, NULL);
  secs += (double)(t_fin.tv_sec + (double)t_fin.tv_usec/1000000) - (double)(t_ini.tv_sec + (double)t_ini.tv_usec/1000000);

  printf(" Has tardado %f segundos\n",secs);
  free(e);
}
