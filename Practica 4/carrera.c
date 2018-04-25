#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include "semaforos.h"

#define MAX_CABALLOS 10
#define MAX_APOSTADORES 100
/**
*@brief file para la clave
*/
#define FILEKEY "/bin/cat"
/**
*@brief key para los semaforos y la memoria compartida
*/
#define KEY 1300

/**
*@brief Mensajes pasados por pipes entre caballo y principal
*/
typedef struct{
  int id;
  int posicion;
  int tirada;
} Caballo_inf;
/**
*@brief Mensaje de cada apostador al gestor de apuestas
*/
typedef struct{
  long mtype;
  char nombre[20];
  double apuesta;
  int caballo;
} Msg_apuesta;
/**
*@brief Memoria compartida entre principal, gestor y monitor
*/
typedef struct{
  int *tiradas;             /*Tiradas de los caballos*/
  int *posiciones;          /*Posiciones de los caballos*/
  int *cotizaciones;        /*Cotizaciones de los caballos*/
  Msg_apuesta *apuestas;    /*Apuestas*/
} Carrera_info;
/**
*@brief Carrera de caballos
*/
void main(){
  int caballos, longitud, apostadores, ventanillas, dinero, i;
  Caballo_info *c_inf;
  Carera_info *carrera

  /*Pedimos por teclado los números necesarios*/
  printf("Introduce el numero de caballos : ");
  scanf("%d",&caballos);
  if(caballos > MAX_CABALLOS ){
    perror("Error en los caballos");
    exit(EXIT_FAILURE);
  }
  printf("\nIntroduce la longitud de la carrera : ");
  scanf("%d",&longitud);

  printf("\nIntroduce el numero de apostadores : ");
  scanf("%d",&apostadores);
  if(apostadores> MAX_APOSTADORES ){
    perror("Error en los apostadores");
    exit(EXIT_FAILURE);
  }
  printf("\nIntroduce el numero de ventanillas: ");
  scanf("%d",&ventanillas);
  printf("\nIntroduce el dinero disponible por apostador : ");
  scanf("%d",&dinero);

  /*Creamos la memoria compartida*/
  key = ftok(FILEKEY, KEY);
  if(key == -1){
      perror("Error de clave");
      exit(EXIT_FAILURE);
  }
  id = shmget(key, sizeof(info_carrera), IPC_CREAT | IPC_EXCL | SHM_R | SHM_W);
  if(id == -1){
      perror("Error en shmget");
      exit(EXIT_FAILURE);
  }
  printf("El id de la memoria es %i\n", id);
  inf = shmat(id, (char*)0, 0);
  if(!inf){
    perror("Error en shmat");
    exit(EXIT_FAILURE);
  }

  int pipes[caballos][2];
  pid_t childpid[caballos];
  for(i = 0 ; i < caballos ; i++){
      if(pipe(pipes[i]) == -1){
        perror("Error	creando	la	tuberia");
        exit(EXIT_FAILURE);
      }
  }

  /*Creamos los hijos, siempre el numero de caballos + 3*/
  for(i = 0 ; i < caballos+3 ; i++){
    if((childpid[i] = fork()) == -1){
      perror("Error	en el fork");
      exit(EXIT_FAILURE);
    }

    /*Padre : Porceso principal*/
    else if(childpid[i] > 0){
      if(i < caballos-1) continue;
      if(principal() == ERROR){
        perror("Error en el gestor de apuestas");
        exit(EXIT_FAILURE);
      }
    }

    /*Hijos*/
    else{
      switch(i){
        /*Proceso gestor de apuestas*/
        case 0:
          if(gestor() == ERROR){
            perror("Error en el gestor de apuestas");
            exit(EXIT_FAILURE);
          }

        /*Proceso monitor*/
        case 1:
          if(monitor() == ERROR){
            perror("Error en el monitor");
            exit(EXIT_FAILURE);
          }

        /*Proceso apostador*/
        case 2:
          if(apostador() == ERROR){
            perror("Error en el apostador");
            exit(EXIT_FAILURE);
          }

        /*Caballos*/
        default:
          if(caballo() == ERROR){
            perror("Error en el caballo");
            exit(EXIT_FAILURE);
          }
      }

      exit(EXIT_SUCCESS);
    }
  }

  /*Liberamos y eliminamos todo y salimos*/

}


int principal(int **pipeIdam int **pipeVuelta, int caballos){
  int i, ultimo = MAX_CABALLOS, primero = 1;
  int *posiciones, *tiradas, *recorridos;

  posiciones = (int*)malloc(sizeof(int)*caballos);
  tiradas = (int*)malloc(sizeof(int)*caballos);
  recorridos = (int*)malloc(sizeof(int)*caballos);
  sleep(30);
  signal()
  for(i = 0; i < MAX_CABALLOS, i++){
      close(pipeIda[i][1]);
      close(pipeVuelta[i][0]);
      posiciones[i] = 0;
      recorridos[i] = 0;
    }
  while(1){
      write(pipeVuelta[i][1], (void*)posiciones[i], sizeof(int));
      read(pipeVuelta[i][0], (void*)tiradas[i], sizeof(int));
      recorridos[i] += tiradas[i];
      if(recorridos[i] < recorridos[ultimo]) {
        posiciones[i] = 0;
        if(recorridos[i] == ultimo = i;
      }
      if(recorridos[primero] <= recorridos[i]) {
        posiciones[i] = 1; 
      }
  }
}

int caballo(int num, int longitud, int *pipeIda, int *pipeVuelta){
  int recorrido = 0, posicion = 0;
  close(pipeIda[0]);
  close(pipeVuelta[1]);

  /*Hasta que comienza la carrera*/
  pause();
  while(recorrido < longitud){
    /*Recibimos la posicion en la que estamos*/
    read(pipeVuelta[0], (void*)posicion, sizeof(int));
    /*Si somos ultimos*/
    if (posicion == 0){
      tirada = aleat_num(1,6);
      recorrido += tirada;
      tirada = aleat_num(1,6);
    }
    /*Si somos primeros*/
    else if (tirada == 1) tirada = aleat_num(1,7);
    else tirada = aleat_num(1,6);
      
    recorrido += tirada;
    /*Le enviamos la tirada al padre*/
    write(pipeIda[1], (void*)tirada, sizeof(int));
  }
  return OK;
}

int monitor(){

}

int apostador(){

}

int gestorApuestas(){

}

/*Función capturadora de la señal*/
void captura(int sennal){
  char key[50];
  int k;
  /*En caso de cancelar el programa eliminamos la zona de memoria compartida y los semaforos*/
  if(sennal == SIGINT){
    k = ftok(FILEKEY, KEY);
    printf("Cancelando carrera %d\n", getpid());
    sprintf(key, "ipcrm -M %d", k);
    system(key);
    sprintf(key, "ipcrm -S %d", k);
    system(key);
    /*Matamos al proceso*/
    kill(getpid(), SIGKILL);
    return;
  }
  return;
}

/*Funcion aleatoria*/
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
