/**
*@brief Ejercicio 13
*@author Lucia Rivas Molina
*@author Daniel Santo-Tomas Lopez
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>

typedef struct{
  int hilo;   /*Identificador del hilo*/
  int mult;   /*Multiplicador de la matriz*/
  int dim;    /*Dimensión de la matriz*/
  int *fila;  /*Puntero a los demás hilos*/
  int m[5][5];/*Matriz*/
} Matriz;

void *multiplica_matriz(Matriz *m){
  int i, j, mult;
  int fila[2];
  if(!m){
    printf("\nError en los parámetros de entrada\n");
    return;
  }
  mult = m->mult;
  /*Multiplicamos la matriz por su multiplicador*/
  for(i = 0, m->fila[m->hilo] = 0; i < m->dim; i++){
    printf("\nHilo %d imprimiendo fila %d : ", m->hilo, i);
    for(j = 0; j < m->dim; j++){
      printf("%d ", m->m[i][j]*mult);
    }
    printf("\n\t- El hilo %d va por la fila %d", m->);
    m->fila[m->hilo]++;
    usleep(200000);
  }
  pthread_exit(NULL);
}

void main(void){
  int dim, i, j;
  pthread_t h[2];
  Matriz *m[2];

  /*Pedimos los datos por teclado*/
  printf("Introduce la dimensión de la matriz cuadrada: ");
  scanf("%d", &dim);
  /*Comprobamos que la dimensión es menor de 5*/
  while(5 < dim){
    printf("\nLa dimensión debe ser menor o igual que 5...\nVuelve a introducirla: ");
    scanf("%d", &dim);
  }

  for(i = 0; i < 2; i++){
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
  for(i = 0; i < 2; i++){
    pthread_create(&h[i], NULL, multiplica_matriz, m[i]);
    usleep(500000);
  }

  /*Realizamos el join y liberamos memoria*/
  for(i = 0; i < 2; i++){
      pthread_join(h[i], NULL);
  }
  for(i = 0; i < 2; i++){
    free(m[i]);
  }
  exit(EXIT_SUCCESS);
}
