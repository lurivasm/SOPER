/**
*@brief Ejercicio3
*@author Lucia Rivas Molina
*@author Daniel Santo-Tomas Lopez
*@date 14/04/2018
*@file ejercicio3.c
*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <string.h>
#include <errno.h>
#include <sys/shm.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>
#include <stdlib.h>
#include "semaforos.h"

/**
*@brief file para la clave
*/
#define FILEKEY "/bin/cat"
/**
*@brief key para los semaforos y la memoria compartida
*/
#define KEY 1300
/**
*@brief tamaño del abecedario y los numeros
*/
#define TAM 36
/**
*@brief Estructura para la memoria compartida
*/
typedef struct{
  char caracter;
  int contador; /*COntador para el padre*/
} Data;
/**
*@brief Funcion del productor que produce una letra
*o numero y la guarda en memoria compartida
*@param semid : id del semaforo
*@param buffer : la memoria compartida
*@return OK o ERROR
*/
int productor(int semid, Data *buffer);
/**
*@brief Funcion del consumidor que lee la memoria
*compartida y muestra el caracter por pantalla
*@param semid : id del semaforo
*@param buffer : la memoria compartida
*@return OK o ERROR
*/
int consumidor(int semid, Data *buffer);
/**
*@brief Funcion que convierte el caracter correspondiente
*al entero i pasado como parámetro, las letras o los números dependiendo del valor de la i
*@param i numero
*@return -1 en caso de error o el caracter correspondiente a i
*/
char caracter(int i);

/**
*@brief Main del ejercicio 3
*@return EXIT_FAILURE o EXIT_SUCCESS
*/
int main(void){
  Data *buffer;
  int id, key, semid;
  unsigned short *sem;
  pid_t pid;

  /*Creamos la zona de memoria compartida*/
  key = ftok(FILEKEY, KEY);
  if(key == -1){
      perror("Error de clave");
      exit(EXIT_FAILURE);
  }
  id = shmget(key, sizeof(int), IPC_CREAT | IPC_EXCL | SHM_R | SHM_W);
  if(id == -1){
      perror("Error en shmget");
      exit(EXIT_FAILURE);
  }
  printf("El id de la memoria es %i\n", id);
  buffer = shmat(id, (char*)0, 0);
  if(!buffer){
    perror("Error en shmat");
    exit(EXIT_FAILURE);
  }

  /*Creamos los semáforos*/
  if (Crear_Semaforo(key, 2, &semid) == ERROR){
  perror("Error en el semaforo");
  exit(EXIT_FAILURE);
  }
  sem = (unsigned short*)malloc(sizeof(short)*2);
  if(!sem){
    Borrar_Semaforo(semid);
    perror("Error en el semaforo");
    exit(EXIT_FAILURE);
    }
  sem[0] = 1;
  sem[1] = 0;
  if (Inicializar_Semaforo(semid, sem) == ERROR){
    Borrar_Semaforo(semid);
    free(sem);
    perror("Error en el semaforo");
    exit(EXIT_FAILURE);
    }

  pid = fork();
  /*Caso de error*/
  if(pid < 0){
    perror("Error en el fork");
    shmdt((char*)buffer);
    shmctl (id, IPC_RMID, (struct shmid_ds *)NULL);
    Borrar_Semaforo(semid);
    free(sem);
    exit(EXIT_FAILURE);
  }

  /*El padre es el consumidor*/
  else if(pid > 0){
    if(consumidor(semid, buffer) == ERROR){
      perror("Error en el consumidor");
      shmdt((char*)buffer);
      shmctl (id, IPC_RMID, (struct shmid_ds *)NULL);
      Borrar_Semaforo(semid);
      free(sem);
      exit(EXIT_FAILURE);
    }
    wait(NULL);
  }

  /*El hijo es el productor*/
  else{
    if(productor(semid, buffer) == ERROR){
      perror("Error en el productor");
      shmdt((char*)buffer);
      shmctl (id, IPC_RMID, (struct shmid_ds *)NULL);
      Borrar_Semaforo(semid);
      free(sem);
      exit(EXIT_FAILURE);
    }
    free(sem);
    exit(EXIT_SUCCESS);
  }

  /*Eliminamos semáforos y liberamos memoria*/
  shmdt((char*)buffer);
  shmctl (id, IPC_RMID, (struct shmid_ds *)NULL);
  Borrar_Semaforo(semid);
  free(sem);
  exit(EXIT_SUCCESS);
}

/*Funcion del productor*/
int productor(int semid, Data *buffer){
  if(!buffer) return ERROR;
  int i;
  /*Hace un down del sem0, crea el caracter, lo guarda y hace up del sem1 para avisar al consumidor*/
  for(i = 0; i < TAM; i++){
    if(Down_Semaforo(semid, 0, SEM_UNDO) == ERROR) return ERROR;
    buffer->contador = i;
    if((buffer->caracter = caracter(i)) == ERROR) return ERROR;
    printf("Productor %c\n", buffer->caracter);
    if(Up_Semaforo(semid, 1, SEM_UNDO) == ERROR) return ERROR;
  }

  return OK;
}

/*Funcion del consumidor*/
int consumidor(int semid, Data *buffer){
  if(!buffer) return ERROR;
  int i = 0;
  /*Hace un down del sem1 esperando al up que realiza el productor y hace un up del sem0*/
  while(i < TAM-1){
    if(Down_Semaforo(semid, 1, SEM_UNDO) == ERROR) return ERROR;
    i = buffer->contador;
    printf("Soy el consumidor e imprimo : %c\n", buffer->caracter);
    if(Up_Semaforo(semid, 0, SEM_UNDO) == ERROR) return ERROR;
  }

  return OK;
}

/*Funcion que devuelve una letra del abecedario o un número del 0 al 9*/
char caracter(int i){
  if (i < 0) return -1;
  return i+65 <= 90 ? i+65 : i+22;
}
