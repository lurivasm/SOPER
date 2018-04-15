/**
*@brief Ejercicio2_solved
*@author Lucia Rivas Molina
*@author Daniel Santo-Tomas Lopez
*@date 14/04/2018
*@file ejercicio2_solved.c
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
*@brief file para la clave de ftok
*/
#define FILEKEY "/bin/cat"
/**
*@brief clave para el ftok
*/
#define KEY 1300
/**
*@brief funcion aleatoria entre 1 y 5 para el sleep de los hijos
*/
#define ALEAT aleat_num(1, 5)
/**
*@brief estructura para la memoria compartida
*/
typedef struct{
 char nombre[80];
 int id;
}info;

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
*@brief Función capturadora de la señal
*@param sennal : señal mandada
* En caso de mandar SIGINT el programa se encarga de borrar
* los semaforos y la memoria compartida
*/
void captura(int sennal);
/**
*@brief Funcion que realiza el padre
*@param semid : semid de los semaforos
*@param n_hijos : numero de hijos que ha creado
*@param : inf : estructura de la memoria compartida
*@return OK o ERROR
*/
int padre(int semid, int n_hijos, info *inf);
/**
*@brief Funcion que realiza el hijo
*@param semid : semid de los semaforos
*@param i : numero de hijo que es
*@param : inf : estructura de la memoria compartida
*@return OK o ERROR
*/
int hijo(int semid, info *inf, int i);
/**
*@brief Main del Ejercicio2_solved
*@return EXIT_FAILURE o EXIT_SUCCESS
*/
int main(void){
    info *inf;
    int id, key, n_hijos, i, semid;
    pid_t pid;
    unsigned short *sem;

    /*Pedimos el número de hijos por teclado*/
    printf("Introduce el número de hijos: ");
    scanf("%d", &n_hijos);
    getchar();
    time(NULL);

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
    inf = shmat(id, (char*)0, 0);
    if(!inf){
      perror("Error en shmat");
      exit(EXIT_FAILURE);
    }
    /*Inicializamos el id de inf a 0*/
    inf->id = 0;

    /*Creamos los semáforos*/
    if (Crear_Semaforo(key, 2, &semid) == ERROR){
      shmdt((char*)inf);
      shmctl (id, IPC_RMID, (struct shmid_ds *)NULL);
      perror("Error en el semaforo");
      exit(EXIT_FAILURE);
    }
    sem = (unsigned short*)malloc(sizeof(short)*2);
    if(!sem){
      Borrar_Semaforo(semid);
      perror("Error en el semaforo");
      shmdt((char*)inf);
      shmctl (id, IPC_RMID, (struct shmid_ds *)NULL);
      exit(EXIT_FAILURE);
      }
    sem[0] = 0;
    sem[1] = 0;
    if (Inicializar_Semaforo(semid, sem) == ERROR){
      Borrar_Semaforo(semid);
      free(sem);
      shmdt((char*)inf);
      shmctl (id, IPC_RMID, (struct shmid_ds *)NULL);
      perror("Error en el semaforo");
      exit(EXIT_FAILURE);
      }

    /*Creamos los hijos y cada uno realiza su función*/
    for(i = 0; i < n_hijos; i++){
        pid = fork();

        /*En caso de error*/
        if(pid < 0){
            perror("Error en el fork");
            shmdt((char*)inf);
            shmctl (id, IPC_RMID, (struct shmid_ds *)NULL);
            Borrar_Semaforo(semid);
            free(sem);
            exit(EXIT_FAILURE);
        }

        /*En el padre imprimimos la informacion por pantalla*/
        else if(pid > 0){
          if(i < n_hijos-1) continue;
          if(padre(semid, n_hijos, inf) == ERROR){
            perror("Error en padre");
            shmdt((char*)inf);
            shmctl (id, IPC_RMID, (struct shmid_ds *)NULL);
            Borrar_Semaforo(semid);
            free(sem);
            exit(EXIT_FAILURE);
          }
          break;
        }

        /*En el hijo pedimos el nombre del cliente y acabamos*/
        else{
          sleep(ALEAT);
          if(hijo(semid, inf, i) == ERROR){
            perror("Error en el hijo");
            shmdt((char*)inf);
            free(sem);
            exit(EXIT_FAILURE);
          }
          /*Liberamos y acabamos*/
          free(sem);
          shmdt((char*)inf);
          exit(EXIT_SUCCESS);
        }
    }

    /*Liberamos la memoria y esperamos a todos los hijos*/
    shmdt((char*)inf);
    shmctl (id, IPC_RMID, (struct shmid_ds *)NULL);
    Borrar_Semaforo(semid);
    free(sem);
    exit(EXIT_SUCCESS);
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

/*Función capturadora de la señal*/
void captura(int sennal){
  char key[50];
  int k;
  /*En caso de cancelar el programa eliminamos la zona de memoria compartida y los semaforos*/
  if(sennal == SIGINT){
    k = ftok(FILEKEY, KEY);
    printf("Cancelando programa %d\n", getpid());
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

/*Funcion que realiza el padre*/
int padre(int semid, int n_hijos, info *inf){
  sigset_t mask;
  int cont = 0, i;

  if(!inf || n_hijos < 0) return ERROR;

  /*Establecemos la señal y la máscara para SIGUSR1 solo para el padre*/
  if(signal(SIGUSR1, captura) == SIG_ERR) return ERROR;
  if(signal(SIGINT, captura) == SIG_ERR) return ERROR;
  if(sigfillset(&mask) == -1) return ERROR;
  if(sigdelset(&mask, SIGUSR1) == -1) return ERROR;
  /*Añadimos también SIGINT por si el usuario quiere cancelar el programa*/
  if(sigdelset(&mask, SIGINT) == -1)return ERROR;

  /*Para cada hijo imprimimos la memoria compartida*/
  while(cont < n_hijos){
    if(Up_Semaforo(semid, 0, SEM_UNDO) == ERROR)return ERROR;
    sigsuspend(&mask);
    if(Down_Semaforo(semid, 1, SEM_UNDO) == ERROR)return ERROR;
    printf("\tPadre : Dado de alta cliente %s con id %d\n", inf->nombre, inf->id);
    cont = inf->id;
  }
  /*Una vez acabado hacemos un up para cada hijo, les esperamos y acabamos*/
  for(i = 0; i < n_hijos; i++) {
    if(Up_Semaforo(semid, 0, SEM_UNDO) == ERROR) return ERROR;
    wait(NULL);
  }
  return OK;
}

/*Funcion que realiza el hijo*/
int hijo(int semid, info *inf, int i){
  if(!inf || i < 0) return ERROR;

  if(Down_Semaforo(semid, 0, SEM_UNDO) == ERROR) return ERROR;
  printf("Introduce el nombre del %d cliente: ", i);
  fgets(inf->nombre, 80, stdin);
  /*Quitamos el \n final para evitar un salto de linea*/
  inf->nombre[strlen(inf->nombre)-1] = '\0';
  inf->id++;
  if(Up_Semaforo(semid, 1, SEM_UNDO) == ERROR) return ERROR;
  /*Avisa al padre y acaba*/
  printf("Proceso %d acabado\n", i);
  if(kill(getppid(), SIGUSR1) == -1) return ERROR;
  if(Down_Semaforo(semid, 0, SEM_UNDO) == ERROR) return ERROR;
  return OK;
}
