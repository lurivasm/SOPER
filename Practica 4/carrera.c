#define  _GNU_SOURCE
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
  int *recorridos;          /*Recorridos de los caballos*/
  int *cotizaciones;        /*Cotizaciones de los caballos*/
  int acabado;              /*Flag para controlar que la carrera ha terminado*/
  Msg_apuesta *apuestas;    /*Apuestas*/
} Carrera_info;

/**
*
*/
void captura(int sennal);
int aleat_num(int inf, int sup);
int principal(int **pipeIda, int **pipeVuelta, int caballos, int longitud, Carrera_info *carrera_info, int semid, pid_t *childpid);
int caballo(int num, int longitud, int *pipeIda, int *pipeVuelta);
int monitor(int caballos, int semid, Carrera_info *carrera_info);
int gestor();
int apostador();
/**
*@brief Carrera de caballos
*/
int main(){
  int caballos, longitud, apostadores, ventanillas, dinero, i, semid, key, id;
  unsigned short *sem;
  int **pipesIda;
  int **pipesVuelta;
  pid_t *childpid;
  Carrera_info *carrera_info;


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

  /*Asignamos manejadores y señales*/
  signal(SIGUSR1, captura);
  signal(SIGUSR2, captura);

  /*Reservamos memoria*/
  carrera_info = (Carrera_info*)malloc(sizeof(Carrera_info));
  if(!carrera_info){
    perror("Error de memoria");
    exit(EXIT_FAILURE);
  }
  /*Creamos la memoria compartida*/
  key = ftok(FILEKEY, KEY);
  if(key == -1){
      perror("Error de clave");
      exit(EXIT_FAILURE);
  }
  id = shmget(key, sizeof(carrera_info), IPC_CREAT | IPC_EXCL | SHM_R | SHM_W);
  if(id == -1){
      perror("Error en shmget");
      exit(EXIT_FAILURE);
  }
  printf("El id de la memoria es %i\n", id);
  carrera_info = shmat(id, (char*)0, 0);
  if(!carrera_info){
    perror("Error en shmat");
    exit(EXIT_FAILURE);
  }

  carrera_info->recorridos = (int*)malloc(sizeof(int)*caballos);
  if(!carrera_info->recorridos){
    perror("Error de clave");
    free(carrera_info);
    exit(EXIT_FAILURE);
  }
  carrera_info->tiradas = (int*)malloc(sizeof(int)*caballos);
  if(!carrera_info->tiradas){
    perror("Error de clave");
    free(carrera_info->recorridos);
    free(carrera_info);
    exit(EXIT_FAILURE);
  }

  carrera_info->cotizaciones = (int*)malloc(sizeof(int)*caballos);
  if(!carrera_info->cotizaciones){
    perror("Error de clave");
    free(carrera_info->recorridos);
    free(carrera_info->tiradas);
    free(carrera_info);
    exit(EXIT_FAILURE);
  }
  carrera_info->apuestas = (Msg_apuesta*)malloc(sizeof(Msg_apuesta)*apostadores);
  if(!carrera_info->apuestas){
    perror("Error de clave");
    free(carrera_info->recorridos);
    free(carrera_info->cotizaciones);
    free(carrera_info->tiradas);
    free(carrera_info);
    exit(EXIT_FAILURE);
  }

  /*Creamos los pipes*/
  pipesIda = (int**)malloc(sizeof(int*)*caballos);
  pipesVuelta = (int**)malloc(sizeof(int*)*caballos);
  childpid = (pid_t*)malloc(sizeof(pid_t)*(caballos+3));
  for(i = 0 ; i < caballos ; i++){
    pipesIda[i] = (int*)malloc(sizeof(int)*2);
    pipesVuelta[i] = (int*)malloc(sizeof(int)*2);
    if(pipe(pipesIda[i]) == -1 || pipe(pipesVuelta[i]) == -1){
      perror("Error	creando	la tuberia");
      shmdt((char*)carrera_info);
      free(carrera_info->recorridos);
      free(carrera_info->tiradas);
      free(carrera_info->apuestas);
      free(carrera_info->cotizaciones);
      free(carrera_info);
      shmctl (id, IPC_RMID, (struct shmid_ds *)NULL);
      exit(EXIT_FAILURE);
    }
  }

  /*Creamos los semáforos, uno para la memoria compartida*/
  if (Crear_Semaforo(key, 1, &semid) == ERROR){
    perror("Error en el semaforo");
    shmdt((char*)carrera_info);
    free(carrera_info->recorridos);
    free(carrera_info->tiradas);
    free(carrera_info->apuestas);
    free(carrera_info->cotizaciones);
    free(carrera_info);
    shmctl (id, IPC_RMID, (struct shmid_ds *)NULL);
    exit(EXIT_FAILURE);
  }
  sem = (unsigned short*)malloc(sizeof(short));
  if(!sem){
    Borrar_Semaforo(semid);
    shmdt((char*)carrera_info);
    free(carrera_info->recorridos);
    free(carrera_info->tiradas);
    free(carrera_info->apuestas);
    free(carrera_info->cotizaciones);
    free(carrera_info);
    shmctl (id, IPC_RMID, (struct shmid_ds *)NULL);
    perror("Error en el semaforo");
    exit(EXIT_FAILURE);
    }
  sem[0] = 1;
  if (Inicializar_Semaforo(semid, sem) == ERROR){
    Borrar_Semaforo(semid);
    free(sem);
    shmdt((char*)carrera_info);
    free(carrera_info->recorridos);
    free(carrera_info->tiradas);
    free(carrera_info->apuestas);
    free(carrera_info->cotizaciones);
    free(carrera_info);
    shmctl (id, IPC_RMID, (struct shmid_ds *)NULL);
    perror("Error en el semaforo");
    exit(EXIT_FAILURE);
    }

  /*Creamos los hijos, siempre el numero de caballos + 3*/
  for(i = 0 ; i < caballos+3 ; i++){
    /*En caso de error en el fork*/
    if((childpid[i] = fork()) == -1){
      perror("Error	en el fork");
      Borrar_Semaforo(semid);
      free(sem);
      shmdt((char*)carrera_info);
      free(carrera_info->recorridos);
      free(carrera_info->tiradas);
      free(carrera_info->apuestas);
      free(carrera_info->cotizaciones);
      free(carrera_info);
      shmctl (id, IPC_RMID, (struct shmid_ds *)NULL);
      exit(EXIT_FAILURE);
    }

    /*Padre : Porceso principal*/
    else if(childpid[i] > 0){
      if(i < caballos+2) continue;
      signal(SIGINT, captura);
      if(principal(pipesIda, pipesVuelta, caballos, longitud, carrera_info, semid, childpid) == ERROR){
        perror("Error en el proceso principal");
        Borrar_Semaforo(semid);
        free(sem);
        shmdt((char*)carrera_info);
        free(carrera_info->recorridos);
        free(carrera_info->tiradas);
        free(carrera_info->apuestas);
        free(carrera_info->cotizaciones);
        free(carrera_info);
        shmctl (id, IPC_RMID, (struct shmid_ds *)NULL);
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
            Borrar_Semaforo(semid);
            free(sem);
            shmdt((char*)carrera_info);
            free(carrera_info->recorridos);
            free(carrera_info->tiradas);
            free(carrera_info->apuestas);
            free(carrera_info->cotizaciones);
            free(carrera_info);
            shmctl (id, IPC_RMID, (struct shmid_ds *)NULL);
            exit(EXIT_FAILURE);
          }
          break;

        /*Proceso monitor*/
        case 1:
          if(monitor(caballos, semid, carrera_info) == ERROR){
            perror("Error en el monitor");
            Borrar_Semaforo(semid);
            free(sem);
            shmdt((char*)carrera_info);
            free(carrera_info->recorridos);
            free(carrera_info->tiradas);
            free(carrera_info->apuestas);
            free(carrera_info->cotizaciones);
            free(carrera_info);
            shmctl (id, IPC_RMID, (struct shmid_ds *)NULL);
            exit(EXIT_FAILURE);
          }
          break;

        /*Proceso apostador*/
        case 2:
          if(apostador() == ERROR){
            perror("Error en el apostador");
            Borrar_Semaforo(semid);
            free(sem);
            shmdt((char*)carrera_info);
            free(carrera_info->recorridos);
            free(carrera_info->tiradas);
            free(carrera_info->apuestas);
            free(carrera_info->cotizaciones);
            free(carrera_info);
            shmctl (id, IPC_RMID, (struct shmid_ds *)NULL);
            exit(EXIT_FAILURE);
          }
          break;

        /*Caballos*/
        default:
          srand(getpid());
          if(caballo(i-3, longitud, pipesIda[i-3], pipesVuelta[i-3]) == ERROR){
            perror("Error en el caballo");
            Borrar_Semaforo(semid);
            free(sem);
            shmdt((char*)carrera_info);
            free(carrera_info->recorridos);
            free(carrera_info->tiradas);
            free(carrera_info->apuestas);
            free(carrera_info->cotizaciones);
            free(carrera_info);
            shmctl (id, IPC_RMID, (struct shmid_ds *)NULL); /*Preguntar si borrar esto en hijos*/
            exit(EXIT_FAILURE);
          }
          break;
      }
      /*Liberamos y salimos*/
    /*  free(sem);
      shmdt((char*)carrera_info);
      free(carrera_info->recorridos);
      free(carrera_info->tiradas);
      free(carrera_info->apuestas);
      free(carrera_info->cotizaciones);
      free(carrera_info);*/
      exit(EXIT_SUCCESS);
    }
  }

  /*Liberamos y eliminamos todo y salimos*/
  Borrar_Semaforo(semid);
  free(sem);
  shmdt((char*)carrera_info);
  free(carrera_info->recorridos);
  free(carrera_info->tiradas);
  free(carrera_info->apuestas);
  free(carrera_info->cotizaciones);
  free(carrera_info);
  shmctl (id, IPC_RMID, (struct shmid_ds *)NULL);
  exit(EXIT_SUCCESS);
}


int principal(int **pipeIda, int **pipeVuelta, int caballos, int longitud, Carrera_info *carrera_info, int semid, pid_t *childpid){
  int i, ultimo = 0, primero = 0, acabado = 0;
  int posiciones[caballos], recorridos[caballos], tiradas[caballos];

  /*Control de errores*/
  if(!pipeIda || !pipeVuelta || !carrera_info || !childpid) return ERROR;
  if(!carrera_info->tiradas || !carrera_info->recorridos) return ERROR;
  /*Inicializamos los pipes, las posiciones y los recorridos*/
  for(i = 0; i < caballos; i++){
      close(pipeIda[i][1]);
      close(pipeVuelta[i][0]);
      posiciones[i] = 2;
      recorridos[i] = 0;
    }

  /*Hasta comenzar la carrera*/
  pause();
  while(acabado != 1){
    /*Enviamos la posicion de cada caballo*/
    for(i = 0; i < caballos; i++) {
      if(recorridos[i] <= recorridos[ultimo] && i != 0) posiciones[ultimo] = 0;
      else if(recorridos[i] >= recorridos[primero] && i != 0) posiciones[primero] = 1;
  /*    printf("Padre envia %d\n", posiciones[i]);*/
      write(pipeVuelta[i][1], &posiciones[i], sizeof(int));
    }
    ultimo = 0;
    primero = 0;
    /*Leemos la tirada y recalculamos*/
    if(Down_Semaforo(semid, 0, SEM_UNDO) == ERROR) return ERROR;
    for(i = 0; i < caballos; i++) {
      read(pipeIda[i][0], &tiradas[i], sizeof(int));
/*      printf("Leo %d %d\n", i, tiradas[i]);*/
      recorridos[i] += tiradas[i];
      if(recorridos[i] <= recorridos[ultimo]) ultimo = i;
      else if(recorridos[i] >= recorridos[primero]) primero = i;
      posiciones[i] = 2;
      carrera_info->recorridos[i] = recorridos[i];
      carrera_info->tiradas[i] = tiradas[i];
  /*    printf("Papa r %d t %d\n\n", carrera_info->recorridos[i], carrera_info->tiradas[i]);*/
      /*Si un caballo alcanza la meta se acaba la carrera*/
      if(longitud <= recorridos[i]) acabado = 1;
    }
    carrera_info->acabado = acabado;
    if(Up_Semaforo(semid, 0, SEM_UNDO) == ERROR) return ERROR;
    /*Avisamos al monitor de que imprima la tirada*/
    kill(childpid[1], SIGUSR1);
  }
  /*Avisamos a los caballos para que acaben*/
  acabado = -1;
  for(i = 0; i < caballos; i++) write(pipeVuelta[i][1], &acabado, sizeof(int));
  pause();
  return OK;
}

/*Funcion de cada caballo*/
int caballo(int num, int longitud, int *pipeIda, int *pipeVuelta){
  int recorrido = 0, posicion = 0, tirada;
  close(pipeIda[0]);
  close(pipeVuelta[1]);

  while(recorrido < longitud){
    /*Recibimos la posicion en la que estamos*/
    read(pipeVuelta[0], &posicion, sizeof(int));
    /*Si recibimos -1 significa que debemos acabar*/
    if(posicion == -1) break;

    /*Si somos ultimos*/
    else if (posicion == 0){
      tirada = aleat_num(1,6);
      recorrido += tirada;
      tirada += aleat_num(1,6);
    }
    /*Si somos primeros*/
    else if (tirada == 1) tirada = aleat_num(1,7);
    else tirada = aleat_num(1,6);

    recorrido += tirada;
  /*  printf("Caballo %d lee %d envia %d\n", num, posicion, tirada);
    /*Le enviamos la tirada al padre*/
    write(pipeIda[1], &tirada, sizeof(int));
  }
  printf("CABALLO ACABADO %d\n", num);
  return OK;
}

/*Función de cada monitor*/
int monitor(int caballos, int semid, Carrera_info *carrera_info){
  int i, acabado = 0;

  for(i = 1; i > 0; i--){
    printf("Quedan %d segundos\n", i*10);
    sleep(10);
  }
  printf("Comienza la carrera!\n");
  /*Avisamos al padre de que comienza la carrera*/
  kill(getppid(), SIGUSR2);
  printf("\t\t");
  for(i = 0; i < caballos; i++) printf("C%d\t", i);
  printf("\n");
  while(acabado != 1){
    /*Espera a que el padre le avise para imprimir*/
    pause();
    if(Down_Semaforo(semid, 0, SEM_UNDO) == ERROR) return ERROR;
    printf("Tiradas   \t");
    for(i = 0; i < caballos; i++) printf("%d\t", carrera_info->tiradas[i]);
    printf("\nRecorrido\t");
    for(i = 0; i < caballos; i++) printf("%d\t", carrera_info->recorridos[i]);
    printf("\n");
    acabado = carrera_info->acabado;
    if(Up_Semaforo(semid, 0, SEM_UNDO) == ERROR) return ERROR;
  }
  printf("La carrera ha finalizado!\n");
  return OK;
}

int apostador(){
  pause();
  return OK;
}

int gestor(){
  pause();
  return OK;
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
