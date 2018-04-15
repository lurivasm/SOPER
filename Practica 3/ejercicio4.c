/**
*@brief Ejercicio4
*@author Lucia Rivas Molina
*@author Daniel Santo-Tomas Lopez
*@date 14/04/2018
*@file ejercicio4.c
*/
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <time.h>

/**
*@brief infimo para el numero aleatorio creado en el fichero
*/
#define INF 100
/**
*@brief supremo para el aleatorio creado en el fichero e infimo
*para el numero de caracteres a escribir
*/
#define MID 1000
/**
*@brief supremo para el aleatorio de numero de caracteres a escribir
*/
#define SUP 2000
/**
*@brief Devuelve un numero aleatorio entre inf y sup
* En caso de pasar un numero negativo se cambia de signo
* En caso de que sup sea menor inf se permutan
*@param inf minimo numero aleatorio que puede salir
*@param sup maximo numero aleatorio que puede salir
*@return el numero aleatorio
*/
int aleat_num(int inf, int sup);
/**
*@brief Función que realiza el hilo 1 abriendo el fichero
*arg y llenandolo con una cantidad aleatoria de numeros aleatorios
*@param arg : path del fichero a abrir
*/
void *hilo1(void *arg);
/**
*@brief Función que realiza el hilo 2 mapeando el fichero modificado
*por el hilo 1 y convirtiendo las comas en espacios
*@param arg : path del fichero a abrir
*/
void *hilo2(void *arg);

/**
*@brief Main del ejercicio 4
*@return EXIT_FAILURE o EXIT_SUCCESS
*/
int main(void){
  char archivo[15] = "ejercicio4.txt";
  pthread_t h1, h2;
  /*Lanzamos hilo 1*/
  pthread_create(&h1, NULL, hilo1, (void*)archivo);
  pthread_join(h1, NULL);
  /*Lanzamos hilo 2*/
  pthread_create(&h2, NULL, hilo2, (void*)archivo);
  pthread_join(h2, NULL);

  exit(EXIT_SUCCESS);
}

/*Funcion del hilo 1*/
void *hilo1(void *arg){
  int len, num, i;
  FILE *fich;

  if(!arg){
    perror("Error en el argumento");
    pthread_exit(NULL);
  }
  /*Abrimos el fichero*/
  fich = fopen((char*)arg, "w");
  if(!fich){
    perror("Error en fopen");
    pthread_exit(NULL);
  }

  /*Establecemos la longitud y escribimos en el fichero*/
  len = aleat_num(MID, SUP);
  for(i = 0; i < len-1; i++){
    num = aleat_num(INF, MID);
    fprintf(fich, "%d,", num);
  }
  /*Escribimos el ultimo numero sin coma*/
  num = aleat_num(INF, MID);
  fprintf(fich, "%d", num);
  /*Cerramos el fichero y salimos*/
  fclose(fich);
  pthread_exit(NULL);
}

/*Funcion del hilo 2*/
void *hilo2(void *arg){
  int fich, size, i, res;
  char *buffer;
  struct stat stat_buff;

  if(!arg){
    perror("Error en el argumento");
    pthread_exit(NULL);
  }
  /*Abrimos el fichero para leer y escribir*/
  fich = open((char*)arg, O_RDWR);
  if(fich < 0){
    perror("Error en fopen");
    pthread_exit(NULL);
  }
  /*Vemos el tamaño*/
  if(fstat(fich, &stat_buff) < 0){
    perror("Error en fstat");
    pthread_exit(NULL);
  }
  size = stat_buff.st_size;

  /*Mapeamos y convertimos las comas en espacios*/
  buffer = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fich, 0);
  if(buffer == MAP_FAILED){
    perror("Error en mmap");
    pthread_exit(NULL);
  }
  printf("Hilo 2 imprime:\n");
  for(i = 0; i < size; i++){
    if(44 == buffer[i]) buffer[i] = 32;
    printf("%c", buffer[i]);
  }
  /*Liberamos y cerramos*/
  res = munmap(0, size);
  if(res == -1){
    perror("Error munmap");
  }
  close(fich);
  pthread_exit(NULL);
}

/*Funcion que devuelve un numero aleatorio entre inf y sup*/
int aleat_num(int inf, int sup){
  int aux;
  /*En caso de que alguno sea negativo los cambia a positivo*/
  if(sup < 0) sup = -sup;
  if(inf < 0) inf = -inf;

  /*En caso de que sup sea menor que inf los permuta*/
  if( sup < inf ){
    aux = sup;
    sup = inf;
    inf = aux;
  }
  return inf+(rand()%(sup-inf+1));
}
