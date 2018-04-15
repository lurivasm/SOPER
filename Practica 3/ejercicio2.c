/**
*@brief Ejercicio2
*@author Lucia Rivas Molina
*@author Daniel Santo-Tomas Lopez
*@date 14/04/2018
*@file ejercicio2.c
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
*/
void captura(int sennal);

/**
*@brief Main del Ejercicio2
*@return EXIT_FAILURE o EXIT_SUCCESS
*/
int main(void){
    info *inf;
    int id, key, n_hijos, i, cont = 0;
    pid_t pid;
    sigset_t mask;

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

    /*Creamos los hijos y cada uno realiza su función*/
    for(i = 0; i < n_hijos; i++){
        pid = fork();

        /*En caso de error*/
        if(pid < 0){
            perror("Error en el fork");
            exit(EXIT_FAILURE);
        }

        /*En el padre*/
        else if(pid > 0){
          if(i < n_hijos-1) continue;
          /*Establecemos la señal y la máscara para SIGUSR1 solo para el padre*/
          if(signal(SIGUSR1, captura) == SIG_ERR){
            perror("Error en signal");
            exit(EXIT_FAILURE);
          }
          if(signal(SIGINT, captura) == SIG_ERR){
            perror("Error en signal");
            exit(EXIT_FAILURE);
          }
          if(sigfillset(&mask) == -1){
            perror("Error en sigfillset");
            exit(EXIT_FAILURE);
          }
          if(sigdelset(&mask, SIGUSR1) == -1){
            perror("Error en sigdelset");
            exit(EXIT_FAILURE);
          }
          /*Añadimos también SIGINT por si el usuario quiere cancelar el programa*/
          if(sigdelset(&mask, SIGINT) == -1){
            perror("Error en sigdelset");
            exit(EXIT_FAILURE);
          }
          for(cont = 0; cont < n_hijos; cont++){
            sigsuspend(&mask);
            printf("\tPadre : Dado de alta cliente %s con id %d\n", inf->nombre, inf->id);
          }
        }

        /*En el hijo*/
        else{
          /*Duerme y pide el nombre por teclado*/
          sleep(ALEAT);
          printf("\nIntroduce el nombre del cliente: ");
          fgets(inf->nombre, 80, stdin);
          /*Quitamos el \n final para evitar un salto de linea*/
          inf->nombre[strlen(inf->nombre)-1] = '\0';
          inf->id++;
          /*Avisa al padre y acaba*/
          kill(getppid(), SIGUSR1);
          shmdt((char*)inf);
          printf("Proceso %d acabado\n", i);
          exit(EXIT_SUCCESS);
        }
    }

    /*Liberamos la memoria*/
    shmdt((char*)inf);
    shmctl (id, IPC_RMID, (struct shmid_ds *)NULL);
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
  /*En caso de cancelar el programa eliminamos la zona de memoria compartida*/
  if(sennal == SIGINT){
    printf("Cancelando programa %d\n", getpid());
    sprintf(key, "ipcrm -M %d", ftok(FILEKEY, KEY));
    system(key);
    kill(getpid(), SIGKILL);
    return;
  }
  return;
}
