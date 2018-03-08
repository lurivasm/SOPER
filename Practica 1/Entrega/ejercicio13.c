/**
*@brief Ejercicio 13
*@author Lucia Rivas Molina <lucia.rivasmolina@estudiante.uam.es>
*@author Daniel Santo-Tomas Lopez <daniel.santo-tomas@estudiante.uam.es>
*@file ejercicio13.c
*@date 2018/03/07
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>

#define NUM_HILOS 2     /*Número de hilos del ejercicio*/
#define RETARDO 500000  /*Tamaño del retardo en ms*/
#define MAX_TAM 5       /*Tamaño máximo de la matriz*/

/**
*@brief Estructura con los datos para multiplicar una matriz por
*       un multiplicador en un hilo
*/
typedef struct{
  int hilo;   /*Identificador del hilo*/
  int mult;   /*Multiplicador de la matriz*/
  int dim;    /*Dimensión de la matriz*/
  int *fila;  /*Puntero a los demás hilos*/
  int m[MAX_TAM][MAX_TAM];/*Matriz*/
} Matriz;

/**
*@brief función que multiplica la matriz m por el multiplicador mult de la estructura Matriz
*@param una estructura Matriz con su id de hilo, multiplicador, dimensión, array de filas y matriz
*/
void *multiplica_matriz(Matriz *m){
  int i, mult;
  if(!m){
    printf("\nError en los parámetros de entrada\n");
    return NULL;
  }
  mult = m->mult;
  /*Multiplicamos la matriz por su multiplicador*/
  for(m->fila[m->hilo] = 0; m->fila[m->hilo] < m->dim; m->fila[m->hilo]++){
    printf("\nHilo %d imprimiendo fila %d : ", m->hilo+1, m->fila[m->hilo]);
    for(i = 0; i < m->dim; i++){
      printf("%d ", m->m[m->fila[m->hilo]][i]*mult);
    }
    for(i = 0; i < NUM_HILOS; i++){
      if(i != m->hilo){
        if(m->fila[i] == m->dim){
          printf("\n\t- El hilo %d ha finzalizado", i+1);
          continue;
        }
        printf("\n\t- El hilo %d va por la fila %d", i+1, m->fila[i]);
      }
    }
    usleep(RETARDO/2);
  }
  pthread_exit(NULL);
}

/**
*Programa que multiplica dos matrices por un multiplicador con hilos
*@brief main del ejercicio13
*/
int main(void){
  int dim, i, j;
  pthread_t h[NUM_HILOS];
  Matriz *m[NUM_HILOS];
  int fila[NUM_HILOS];  /*Array de ints para controlar por qué fila va cada hilo*/

  /*Pedimos los datos por teclado*/
  printf("Introduce la dimensión de la matriz cuadrada: ");
  scanf("%d", &dim);
  /*Comprobamos que la dimensión es menor de 5*/
  while(MAX_TAM < dim){
    printf("\nLa dimensión debe ser menor o igual que 5...\nVuelve a introducirla: ");
    scanf("%d", &dim);
  }

  for(i = 0; i < NUM_HILOS; i++){
    m[i] = (Matriz*)malloc(sizeof(Matriz));
    if(!m[i]){
      printf("Error de memoria\n");
      for(j = 0; j < i; j++){
        free(m[j]);
      }
      exit(EXIT_FAILURE);
    }
    m[i]->hilo = i; /*La matriz 1 se la asignamos al hilo 1 y la 2 al 2*/
    m[i]->dim = dim;
    fila[i] = 0;    /*Inicializamos las filas a 0*/
    m[i]->fila = fila;
  }


  printf("\nIntroduce el primer multiplicador: ");
  scanf("%d", &m[0]->mult);
  printf("\nIntroduce el segundo multiplicador: ");
  scanf("%d", &m[1]->mult);

  printf("\nIntroduce la matriz 1: ");
  for(i = 0; i < dim; i++){
    for(j = 0; j < dim; j++){
      fscanf(stdin, "%d", &m[0]->m[i][j]);
    }
  }
  printf("\nIntroduce la matriz 2: ");
  for(i = 0; i < dim; i++){
    for(j = 0; j < dim; j++){
      fscanf(stdin, "%d", &m[1]->m[i][j]);
    }
  }
  printf("\nComenzamos el producto...");


  /*Comenzamos el producto de los hilos*/
  for(i = 0; i < NUM_HILOS; i++){
    pthread_create(&h[i], NULL, (void * (*)(void *))multiplica_matriz, m[i]);
    usleep(RETARDO);
  }

  /*Realizamos el join y liberamos memoria*/
  for(i = 0; i < NUM_HILOS; i++){
      pthread_join(h[i], NULL);
  }
  for(i = 0; i < NUM_HILOS; i++){
    free(m[i]);
  }
  printf("\nEl programa ha finalizado con éxito\n");
  exit(EXIT_SUCCESS);
}
