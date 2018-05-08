#define  _GNU_SOURCE
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <pthread.h>
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

typedef struct{
  int id_cola;         /*id de la cola de mensajes*/
  int semid;           /*id de los semaforos*/
  int id_mem;          /*id de la memoria compartida*/
} Ids_carrera;
/**
*@brief Mensaje de cada apostador al gestor de apuestas
*/
typedef struct{
  long mtype;         /*Tipo de mensaje (por defecto será 1)*/
  char nombre[20];    /*Nombre del apostador*/
  double apuesta;     /*Apuesta que realiza*/
  int caballo;        /*Caballo por el que apuesta*/
  int num;            /*Numero de apostador*/
} Msg_apuesta;
/**
*@brief Memoria compartida entre principal, gestor y monitor
*/
typedef struct{
  int id_cola;         /*id de la cola de mensajes*/
  int semid;           /*id de los semaforos*/
  int tiradas[MAX_CABALLOS];             /*Tiradas de los caballos*/
  int recorridos[MAX_CABALLOS];          /*Recorridos de los caballos*/
  int acabado;                           /*Flag para controlar que la carrera ha terminado*/
  double cotizacion[MAX_CABALLOS];        /*cotizacion de los caballos*/
  double pago[MAX_APOSTADORES];             /*Pago al apostador en caso de que gane*/
  int total_apuestas;                       /*Total apostado a todos los caballos*/
  int total_caballo[MAX_CABALLOS];          /*Total apostado a cada caballo*/
  int cont;                                 /*Contador de cuantas apuestas se realizan*/
  Msg_apuesta mensaje[MAX_APOSTADORES];     /*Mensajes de apuestas*/
} Carrera_info;

void *ventanilla(int id){
  int caballo, semid, id_cola;
  Msg_apuesta mensaje;
  pthread_mutex_t mutex;
  /*Cogemos la memoria compartida*/
  Carrera_info *info = shmat(id, (char*)0, 0);
  if(!info) return NULL;

  semid = info->semid;
  id_cola = info->id_cola;
  /*Recibimos un mensaje*/
  while(msgrcv(id_cola, (struct msgbuf *)&mensaje, sizeof(Msg_apuesta) - sizeof(long) - sizeof(int), 1, MSG_NOERROR) > 0){
    pthread_mutex_lock(&mutex);
    if(Down_Semaforo(semid, 2, SEM_UNDO) == ERROR) return NULL;
    printf("Ventanilla %d\n", info->cont);
    info->mensaje[info->cont] = mensaje;
    info->total_apuestas += mensaje.apuesta;
    info->total_caballo[mensaje.caballo] += mensaje.apuesta;
    info->pago[mensaje.num] = mensaje.apuesta*info->cotizacion[mensaje.caballo];
    info->cotizacion[mensaje.caballo] = info->total_apuestas/info->total_caballo[mensaje.caballo];
    info->cont++;
    if(Up_Semaforo(semid, 2, SEM_UNDO) == ERROR) return NULL;
    pthread_mutex_unlock(&mutex);
  }
  shmdt((char*)info);
  return NULL;
}
/**
*
*/
void captura(int sennal);
int aleat_num(int inf, int sup);
int principal(sigset_t mask, int **pipeIda, int **pipeVuelta, int caballos, int longitud, int semid, Carrera_info *info, pid_t *childpid);
int caballo(int num, int longitud, int *pipeIda, int *pipeVuelta);
int monitor(sigset_t mask, int caballos, int longitud, int semid, Carrera_info *info);
/**
* Proceso apostador
*/
int apostador(int dinero, int caballos, int apostadores, int id_cola){
  int i;
  Msg_apuesta mensaje;

  /*Control de errores*/
  if(caballos < 0 || apostadores < 0) return ERROR;
  mensaje.mtype = 1;
  /*Enviamos un mensaje por cada apostador*/
  for(i = 0; i < apostadores; i++){
    sprintf(mensaje.nombre, "Apostador-%d", i+1);
    mensaje.apuesta = aleat_num(1, dinero);
    mensaje.caballo = aleat_num(0, caballos-1);
    mensaje.num = i;
    printf("Apostador %d mandando %d %d\n\n", i, mensaje.apuesta, mensaje.caballo);
    if(msgsnd(id_cola, (struct msgbuf *)&mensaje, sizeof(Msg_apuesta) -sizeof(long) -sizeof(int), 0) < 0)return ERROR;
    sleep(1);
  }
  return OK;
}

/**
* Proceso gestor de apuestas
*/
int gestor(int id_cola, int caballos, int apostadores, int ventanillas, int id){
  pthread_t hilo[ventanillas];
  int i;
  int total_caballo[caballos];
  /*Cogemos la memoria compartida*/
  Carrera_info *info = shmat(id, (char*)0, 0);
  if(!info) return ERROR;
  printf("Gestor inicializa\n");
  /*Inicializamos las apuestas de los caballos, las cotizacion y el total*/
  info->cont = 0;
  info->total_apuestas = 0;
  for(i = 0; i < caballos; i++){
    info->total_caballo[i] = 1;
    info->total_apuestas += total_caballo[i];
    info->cotizacion[i] = info->total_apuestas/total_caballo[i];
    printf("%d\n", i);
  }
  /*Inicializamos el pago a los apostadores y nos desprendemos de la memoria compartida*/
  for(i = 0; i < apostadores; i++) info->pago[i] = 0;
  shmdt((char*)info);
  /*Inicializamos las ventanillas de apuestas*/
  printf("Gestor crea hilos\n");
  for(i = 0; i < ventanillas; i++) pthread_create(&hilo[i], NULL , ventanilla, (void*)id);
  for(i = 0; i < ventanillas; i++){printf("esperando\n"); pthread_join(hilo[i], NULL);}
  printf("Gestor muere\n");
  return OK;
}
/**
*@brief Carrera de caballos
*/
int main(){
  int caballos, longitud, apostadores, ventanillas, dinero, i, semid, key, id, id_cola;
  unsigned short *sem;
  int **pipesIda;
  int **pipesVuelta;
  pid_t *childpid;
  sigset_t mask;
  Carrera_info *carrera_info;
  Ids_carrera ids;

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
  if(signal(SIGUSR1, captura) == SIG_ERR) return ERROR;
  if(signal(SIGUSR2, captura) == SIG_ERR) return ERROR;
  if(signal(SIGALRM, captura) == SIG_ERR) return ERROR;
  if(sigfillset(&mask) == -1) return ERROR;
  if(sigdelset(&mask, SIGUSR1) == -1) return ERROR;
  if(sigdelset(&mask, SIGALRM) == -1) return ERROR;
  if(sigdelset(&mask, SIGUSR2) == -1) return ERROR;

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
      shmctl (id, IPC_RMID, (struct shmid_ds *)NULL);
      exit(EXIT_FAILURE);
    }
  }

  /*Creamos los semáforos, uno para la memoria compartida*/
  if (Crear_Semaforo(key, 3, &semid) == ERROR){
    perror("Error en el semaforo");
    shmdt((char*)carrera_info);
    shmctl (id, IPC_RMID, (struct shmid_ds *)NULL);
    exit(EXIT_FAILURE);
  }
  sem = (unsigned short*)malloc(sizeof(short)*3);
  if(!sem){
    Borrar_Semaforo(semid);
    shmdt((char*)carrera_info);
    shmctl (id, IPC_RMID, (struct shmid_ds *)NULL);
    perror("Error en el semaforo");
    exit(EXIT_FAILURE);
    }
  sem[0] = 0;
  sem[1] = 1;
  sem[2] = 1;
  if (Inicializar_Semaforo(semid, sem) == ERROR){
    Borrar_Semaforo(semid);
    free(sem);
    shmdt((char*)carrera_info);
    shmctl (id, IPC_RMID, (struct shmid_ds *)NULL);
    perror("Error en el semaforo");
    exit(EXIT_FAILURE);
    }

  /*Creamos la cola de mensajes para apostadores*/
  id_cola = msgget(key, IPC_CREAT | 0660);
  if(id_cola == -1){
    perror("Error de clave");
    exit(EXIT_FAILURE);
  }
  carrera_info->id_cola = id_cola;
  carrera_info->semid = semid;

  /*Creamos los hijos, siempre el numero de caballos + 3*/
  for(i = 0 ; i < caballos+3 ; i++){
    /*En caso de error en el fork*/
    if((childpid[i] = fork()) == -1){
      perror("Error	en el fork");
      Borrar_Semaforo(semid);
      free(sem);
      shmdt((char*)carrera_info);
      shmctl (id, IPC_RMID, (struct shmid_ds *)NULL);
      msgctl (id_cola, IPC_RMID, (struct msqid_ds *)NULL);
      exit(EXIT_FAILURE);
    }

    /*Padre : Porceso principal*/
    else if(childpid[i] > 0){
      if(i < caballos+2) continue;
      /*Añadimos también SIGINT por si el usuario quiere cancelar el programa*/
      if(signal(SIGINT, captura) == SIG_ERR) return ERROR;
      if(sigdelset(&mask, SIGINT) == -1)return ERROR;
      if(principal(mask, pipesIda, pipesVuelta, caballos, longitud, semid, carrera_info, childpid) == ERROR){
        perror("Error en el proceso principal");
        Borrar_Semaforo(semid);
        free(sem);
        shmdt((char*)carrera_info);
        shmctl (id, IPC_RMID, (struct shmid_ds *)NULL);
        msgctl (id_cola, IPC_RMID, (struct msqid_ds *)NULL);
        exit(EXIT_FAILURE);
      }
    }

    /*Hijos*/
    else{
      switch(i){
        /*Proceso gestor de apuestas*/
        case 0:
          if(gestor(id_cola, caballos, apostadores, ventanillas, id) == ERROR){
            perror("Error en el gestor de apuestas");
            Borrar_Semaforo(semid);
            free(sem);
            shmdt((char*)carrera_info);
            shmctl (id, IPC_RMID, (struct shmid_ds *)NULL);
            msgctl (id_cola, IPC_RMID, (struct msqid_ds *)NULL);
            exit(EXIT_FAILURE);
          }
          break;

        /*Proceso monitor*/
        case 1:
          if(monitor(mask, caballos, longitud, semid, carrera_info) == ERROR){
            perror("Error en el monitor");
            Borrar_Semaforo(semid);
            free(sem);
            shmdt((char*)carrera_info);
            shmctl (id, IPC_RMID, (struct shmid_ds *)NULL);
            msgctl (id_cola, IPC_RMID, (struct msqid_ds *)NULL);
            exit(EXIT_FAILURE);
          }
          break;

        /*Proceso apostador*/
        case 2:
          free(sem);
          shmdt((char*)carrera_info);
          srand(getpid());
          if(apostador(dinero, caballos, apostadores, id_cola) == ERROR){
            perror("Error en el apostador");
            Borrar_Semaforo(semid);
            free(sem);
            shmdt((char*)carrera_info);
            shmctl (id, IPC_RMID, (struct shmid_ds *)NULL);
            msgctl (id_cola, IPC_RMID, (struct msqid_ds *)NULL);
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
            shmctl (id, IPC_RMID, (struct shmid_ds *)NULL); /*Preguntar si borrar esto en hijos*/
            msgctl (id_cola, IPC_RMID, (struct msqid_ds *)NULL);
            exit(EXIT_FAILURE);
          }
          break;
      }
      /*Liberamos y salimos*/
      free(sem);
      shmdt((char*)carrera_info);
      exit(EXIT_SUCCESS);
    }
  }

  /*Liberamos y eliminamos todo y salimos*/
  for(i = 0; i < caballos+3; i++) wait(NULL);
  free(sem);
  Borrar_Semaforo(semid);
  shmdt((char*)carrera_info);
  shmctl (id, IPC_RMID, (struct shmid_ds *)NULL);
  msgctl (id_cola, IPC_RMID, (struct msqid_ds *)NULL);
  exit(EXIT_SUCCESS);
}


int principal(sigset_t mask, int **pipeIda, int **pipeVuelta, int caballos, int longitud, int semid, Carrera_info *info, pid_t *childpid){
  int i, ultimo = 0, primero = 0, acabado = 0;
  int posiciones[caballos], recorridos[caballos], tiradas[caballos];

  /*Control de errores*/
  if(!pipeIda || !pipeVuelta || !info || !childpid) return ERROR;
  if(!info->tiradas || !info->recorridos) return ERROR;
  /*Inicializamos los pipes, las posiciones y los recorridos*/
  for(i = 0; i < caballos; i++){
      close(pipeIda[i][1]);
      close(pipeVuelta[i][0]);
      posiciones[i] = 2;
      recorridos[i] = 0;
    }

  /*Hasta comenzar la carrera*/
  sigsuspend(&mask);
  /*Matamos a los apostadores y al gestor de apuestas*/
  kill(childpid[2], SIGUSR2);
  kill(childpid[0], SIGUSR2);

  while(acabado != 1){
    /*Enviamos la posicion de cada caballo*/
    for(i = 0; i < caballos; i++) {
      if(recorridos[i] <= recorridos[ultimo] && i != 0) posiciones[ultimo] = 0;
      else if(recorridos[i] >= recorridos[primero] && i != 0) posiciones[primero] = 1;
      write(pipeVuelta[i][1], &posiciones[i], sizeof(int));
    }
    ultimo = 0;
    primero = 0;

    /*Leemos la tirada y recalculamos*/
    if(Down_Semaforo(semid, 1, SEM_UNDO) == ERROR) return ERROR;

    for(i = 0; i < caballos; i++) {
      read(pipeIda[i][0], &tiradas[i], sizeof(int));
      recorridos[i] += tiradas[i];
      if(recorridos[i] <= recorridos[ultimo]) ultimo = i;
      else if(recorridos[i] >= recorridos[primero]) primero = i;
      posiciones[i] = 2;
      info->recorridos[i] = recorridos[i];
      info->tiradas[i] = tiradas[i];

      /*Si un caballo alcanza la meta se acaba la carrera*/
      if(longitud <= recorridos[i]){
        acabado = 1;
        info->recorridos[i] = longitud;
      }
    }
    info->acabado = acabado;
    /*Up del semaforo 0 para que el monitor imprima*/
    printf("up %d\n",Up_Semaforo(semid, 0, SEM_UNDO) );
    printf("papa\n");
  }
  /*Avisamos a los caballos para que acaben*/
  acabado = -1;
  for(i = 0; i < caballos; i++) write(pipeVuelta[i][1], &acabado, sizeof(int));
  sigsuspend(&mask);
  return OK;
}

/*Funcion de cada caballo*/
int caballo(int num, int longitud, int *pipeIda, int *pipeVuelta){
  int recorrido = 0, posicion = 0, tirada;
  close(pipeIda[0]);
  close(pipeVuelta[1]);

  while(1){
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

    /*Le enviamos la tirada al padre*/
    write(pipeIda[1], &tirada, sizeof(int));
  }
  return OK;
}

/*Función de cada monitor*/
int monitor(sigset_t mask, int caballos, int longitud, int semid, Carrera_info *info){
  int i, acabado = 0;

  for(i = 1; i > 0; i--){
    printf("Quedan %d segundos\n", i*10);
    sleep(7);
  }
  printf("\nComienza la carrera!\n\n");
  /*Avisamos al padre de que comienza la carrera*/
  kill(getppid(), SIGUSR1);
  printf("\t\t");
  for(i = 0; i < caballos; i++) printf("C%d\t", i);
  printf("\n");
  while(acabado != 1){
    printf("monitor\n");
    /*Espera a que el padre le avise para imprimir*/
    printf("down %d", Down_Semaforo(semid, 0, SEM_UNDO));
    printf("Tiradas   \t");
    for(i = 0; i < caballos; i++) printf("%d\t", info->tiradas[i]);
    printf("\nRecorrido\t");
    for(i = 0; i < caballos; i++) printf("%d\t", info->recorridos[i]);
    printf("\n");
    acabado = info->acabado;
    if(Up_Semaforo(semid, 1, SEM_UNDO) == ERROR) return ERROR;
  }
  printf("\nLa carrera ha finalizado! En breves daremos el report\n\n");

  /*Esperamos 15 segundos hasta realizar el report*/
  alarm(5);
  sigsuspend(&mask);
  printf("\t\t ______________________ \n");
  printf("\t\t|                      |\n");
  printf("\t\t| REPORT DE LA CARRERA |\n");
  printf("\t\t|______________________|\n\n");
  /*Imprimimos las apuestas realizadas*/
  printf("\tApuestas realizadas : \n");
  /*for(i = 0; i < info->cont; i++) {
    printf("\t - %s : %d euros al ", info->mensaje[i].nombre, info->mensaje[i].apuesta);
    printf("Caballo %d () -> Ventanilla\n", info->mensaje[i].caballo);
  }
  /*Imprimimos el resultado de los caballos*/
  printf("Resultados de la carrera : \n");
  for(i = 0; i < caballos; i++) {
    printf("\t - Caballo %d : %d metros recorridos", i+1, info->recorridos[i]);
    if(info->recorridos[i] == longitud) printf(" 1º Posición\n");
    else {printf("\n");}
  }
  /*Avisamos al padre de que hemos acabado y salimos*/
  kill(getppid(), SIGUSR1);
  return OK;
}




/**
* Función capturadora de la señal
*/
void captura(int sennal){
  char key[50];
  int k;
  if(sennal == SIGUSR2){
    exit(EXIT_SUCCESS);
  }
  /*En caso de cancelar el programa eliminamos la zona de memoria compartida y los semaforos*/
  else if(sennal == SIGINT){
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

/**
* Funcion aleatoria
*/
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
