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

#define FILEKEY "/bin/cat"
#define KEY 1300
#define ALEAT aleat_num(1, 5)

typedef struct{
 char nombre[80];
 int id;
}info;

int aleat_num(int inf, int sup);
void captura(int sennal){
  printf("Padre recibiendo sennal SIGUSR1\n");
  return;
}

int main(void){
    info *inf;
    int id, key, n_hijos, i, cont = 0;
    pid_t pid;

    /*Establecemos la señal y el time para el rand*/
    time(NULL);
    signal(SIGUSR1, captura);

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
          while(cont < n_hijos){
            pause();
            printf("Padre imprimiendo...\n");
            printf("Cliente %s id %d\n", inf->nombre, inf->id);
            cont++;
          }
        }

        /*En el hijo*/
        else{
          sleep(ALEAT);
          printf("Introduce el nombre del cliente: ");
          fgets(inf->nombre, 80, stdin);
          printf("Aumentando en 1 el número de usuarios\n");
          inf->id++;
          /*Avisa al padre y acaba*/
          kill(getppid(), SIGUSR1);
          printf("Proceso %d acabado\n", i);
          exit(EXIT_SUCCESS);
        }
    }

    /*Liberamos la memoria*/
    shmdt((char*)inf);
    shmctl (id, IPC_RMID, (struct shmid_ds *)NULL);
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
