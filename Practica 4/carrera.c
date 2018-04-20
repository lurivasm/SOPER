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
  int caballos, longitud, apostadores, ventanillas, dinero,i;
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

    /*Papa*/
    else if(childpid[i] > 0){
      if(i < caballos-1) continue;
      if(principal() == ERROR){
        perror("Error en el gestor de apuestas");
        exit(EXIT_FAILURE);
      }
    }

    /*Hijo*/
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


int principal(){
  sleep(30);
  signal()
}

int caballo(int num, int longitud, int *pipeIda, int *pipeVuelta){
  Caballo_inf info;
  int recorrido = 0;

  info.id = num;
  info.posicion = 0;
  close(pipeIda[0]);
  close(pipeVuelta[1]);

  /*Hasta que comienza la carrera*/
  pause();
  while(recorrido < longitud){
    /*Recibimos la posicion en la que estamos*/
    read(pipeVuelta[0], (void*)info.posicion, sizeof(int));
    /*Si somos ultimos*/
    if (info.posicion == 0){
      info.tirada = aleat_num(1,6);
      recorrido += info.tirada;
      info.tirada = aleat_num(1,6);
      recorrido += info.tirada;
    }
    /*Si somos primeros*/
    else if (info.tirada == 1){
      info.tirada = aleat_num(1,7);
      info.recorrido += info.tirada;
    }
    else{
      info.tirada = aleat_num(1,6);
      info.recorrido += info.tirada;
    }
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
