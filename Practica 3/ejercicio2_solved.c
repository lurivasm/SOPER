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

#define FILEKEY "/bin/cat"
#define KEY 1300
#define ALEAT aleat_num(1, 5)
/**
*@brief key para los semaforos
*/
#define SEMKEY 75798

typedef struct{
 char nombre[80];
 int id;
}info;

int aleat_num(int inf, int sup);
void captura(){
  return;
}

int main(void){
    info *inf;
    int id, key, n_hijos, i, semid;
    pid_t pid;
    unsigned short *sem;

    /*Establecemos la señal y el time para el rand*/
    time(NULL);
    if(signal(SIGUSR1, captura) == SIG_ERR){
      perror("Error en signal");
      exit(EXIT_FAILURE);
    }

    /*Pedimos el número de hijos por teclado*/
    printf("Introduce el número de hijos: ");
    scanf("%d", &n_hijos);
    getchar();

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
    if (Crear_Semaforo(SEMKEY, 1, &semid) == ERROR){
    perror("Error en el semaforo");
    exit(EXIT_FAILURE);
    }
    sem = (unsigned short*)malloc(sizeof(short));
    if(!sem){
      Borrar_Semaforo(semid);
      perror("Error en el semaforo");
      exit(EXIT_FAILURE);
      }
    *sem = 1;
    if (Inicializar_Semaforo(semid, sem) == ERROR){
      Borrar_Semaforo(semid);
      free(sem);
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

        /*En el padre*/
        else if(pid > 0){
          if(i < n_hijos-1) continue;
          while(inf->id < n_hijos){
            pause();
            if(Down_Semaforo(semid, 0, SEM_UNDO) == ERROR){
              perror("Error en los semaforos");
              Borrar_Semaforo(semid);
              free(sem);
              exit(EXIT_FAILURE);
            }
            printf("Padre imprimiendo...\n");
            printf("\tCliente %s\n\tid %d\n", inf->nombre, inf->id);
            if(Up_Semaforo(semid, 0, SEM_UNDO) == ERROR){
              shmdt((char*)inf);
              shmctl (id, IPC_RMID, (struct shmid_ds *)NULL);
              perror("Error en los semaforos");
              Borrar_Semaforo(semid);
              free(sem);
              exit(EXIT_FAILURE);
            }
          }
          break;
        }

        /*En el hijo*/
        else{
          sleep(ALEAT);
          if(Down_Semaforo(semid, 0, SEM_UNDO) == ERROR){
            perror("Error en los semaforos");
            Borrar_Semaforo(semid);
            free(sem);
            exit(EXIT_FAILURE);
          }
          printf("Introduce el nombre del cliente: ");
          fgets(inf->nombre, 80, stdin);
          inf->nombre[strlen(inf->nombre)-2] = '\0';
          printf("Aumentando en 1 el número de usuarios\n");
          inf->id++;
          /*Avisa al padre y acaba*/
          kill(getppid(), SIGUSR1);
          if(Up_Semaforo(semid, 0, SEM_UNDO) == ERROR){
            perror("Error en los semaforos");
            Borrar_Semaforo(semid);
            free(sem);
            exit(EXIT_FAILURE);
          }
          printf("Proceso %d acabado\n", i);
          exit(EXIT_SUCCESS);
        }
    }

    /*Liberamos la memoria y esperamos a todos los hijos*/
    for(i = 0; i < n_hijos; i++) wait(NULL);
    shmdt((char*)inf);
    shmctl (id, IPC_RMID, (struct shmid_ds *)NULL);
    Borrar_Semaforo(semid);
    free(sem);
    exit(EXIT_SUCCESS);
}


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
