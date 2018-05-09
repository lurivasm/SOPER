#define  _GNU_SOURCE
#include "lib.h"

void libera_memoria(int id_cola, int id, int semid, unsigned short* sem, int **pipesIda, int **pipesVuelta, pid_t *pid);
/**
*@brief Carrera de caballos
* En el main se reserva y libera la memoria, además de lanzar los hijos
*/
int main(){
  int caballos, longitud, apostadores, ventanillas, dinero, i, semid, key, id, id_cola, j;
  unsigned short *sem;
  int **pipesIda;
  int **pipesVuelta;
  pid_t *childpid;
  sigset_t mask;
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
  if(apostadores > MAX_APOSTADORES ){
    perror("Error en los apostadores");
    exit(EXIT_FAILURE);
  }
  printf("\nIntroduce el numero de ventanillas: ");
  scanf("%d",&ventanillas);
  printf("\nIntroduce el dinero disponible por apostador : ");
  scanf("%d",&dinero);

  /*Asignamos manejadores y señales*/
  if(signal(SIGUSR1, captura) == SIG_ERR) exit(EXIT_FAILURE);
  if(signal(SIGUSR2, captura) == SIG_ERR) exit(EXIT_FAILURE);
  if(signal(SIGALRM, captura) == SIG_ERR) exit(EXIT_FAILURE);
  if(sigfillset(&mask) == -1) exit(EXIT_FAILURE);
  if(sigdelset(&mask, SIGUSR1) == -1) exit(EXIT_FAILURE);
  if(sigdelset(&mask, SIGALRM) == -1) exit(EXIT_FAILURE);
  if(sigdelset(&mask, SIGUSR2) == -1) exit(EXIT_FAILURE);

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
    Borrar_Semaforo(semid);
    free(sem);
    shmdt((char*)carrera_info);
    shmctl (id, IPC_RMID, (struct shmid_ds *)NULL);
    perror("Error de clave");
    exit(EXIT_FAILURE);
  }
  carrera_info->id_cola = id_cola;
  carrera_info->semid = semid;

  /*Creamos un array para los pids*/
  childpid = (pid_t*)malloc(sizeof(pid_t)*(caballos+3));
  if(!childpid){
    perror("Error	memoria");
    Borrar_Semaforo(semid);
    free(sem);
    shmdt((char*)carrera_info);
    shmctl (id, IPC_RMID, (struct shmid_ds *)NULL);
    msgctl (id_cola, IPC_RMID, (struct msqid_ds *)NULL);
    exit(EXIT_FAILURE);
  }

  /*Creamos los pipes*/
  pipesIda = (int**)malloc(sizeof(int*)*caballos);
  if(!pipesIda){
    perror("Error	memoria");
    Borrar_Semaforo(semid);
    free(sem);
    free(childpid);
    shmdt((char*)carrera_info);
    shmctl (id, IPC_RMID, (struct shmid_ds *)NULL);
    msgctl (id_cola, IPC_RMID, (struct msqid_ds *)NULL);
    exit(EXIT_FAILURE);
  }
  pipesVuelta = (int**)malloc(sizeof(int*)*caballos);
  if(!pipesVuelta){
    perror("Error	memoria");
    free(pipesIda);
    free(childpid);
    Borrar_Semaforo(semid);
    free(sem);
    shmdt((char*)carrera_info);
    shmctl (id, IPC_RMID, (struct shmid_ds *)NULL);
    msgctl (id_cola, IPC_RMID, (struct msqid_ds *)NULL);
    exit(EXIT_FAILURE);
  }
  for(i = 0 ; i < caballos ; i++){
    pipesIda[i] = (int*)malloc(sizeof(int)*2);
    pipesVuelta[i] = (int*)malloc(sizeof(int)*2);
    if(pipe(pipesIda[i]) == -1 || pipe(pipesVuelta[i]) == -1){
      perror("Error	creando	la tuberia");
      shmdt((char*)carrera_info);
      shmctl (id, IPC_RMID, (struct shmid_ds *)NULL);
      msgctl (id_cola, IPC_RMID, (struct msqid_ds *)NULL);
      for(j = i; 0 <= j; j--){
        free(pipesIda[j]);
        free(pipesVuelta[j]);
      }
      free(pipesIda);
      free(pipesVuelta);
      exit(EXIT_FAILURE);
    }
  }

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
      for(i = 0; i <caballos; i++){
        free(pipesIda[i]);
        free(pipesVuelta[i]);
      }
      free(pipesIda);
      free(pipesVuelta);
      free(childpid);
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
        for(i = 0; i <caballos; i++){
          free(pipesIda[i]);
          free(pipesVuelta[i]);
        }
        free(pipesIda);
        free(pipesVuelta);
        free(childpid);
        exit(EXIT_FAILURE);
      }
    }

    /*Hijos*/
    else{
      switch(i){
        /*Proceso gestor de apuestas*/
        case 0:
          for(i = 0; i <caballos; i++){
            free(pipesIda[i]);
            free(pipesVuelta[i]);
          }
          free(pipesIda);
          free(pipesVuelta);
          free(childpid);
          free(sem);
          if(gestor(id_cola, caballos, apostadores, ventanillas, id) == ERROR){
            perror("Error en el gestor de apuestas");
            free(sem);
            shmdt((char*)carrera_info);
            exit(EXIT_FAILURE);
          }
          break;

        /*Proceso monitor*/
        case 1:
          if(monitor(mask, caballos, longitud, semid, carrera_info, dinero) == ERROR){
            perror("Error en el monitor");
            free(sem);
            shmdt((char*)carrera_info);
            for(i = 0; i < caballos; i++){
              free(pipesIda[i]);
              free(pipesVuelta[i]);
            }
            free(pipesIda);
            free(pipesVuelta);
            free(childpid);
            exit(EXIT_FAILURE);
          }
          break;

        /*Proceso apostador*/
        case 2:
          free(sem);
          shmdt((char*)carrera_info);
          for(i = 0; i <caballos; i++){
            free(pipesIda[i]);
            free(pipesVuelta[i]);
          }
          free(pipesIda);
          free(pipesVuelta);
          free(childpid);
          srand(getpid());
          if(apostador(dinero, caballos, apostadores, id_cola) == ERROR){
            perror("Error en el apostador");
            exit(EXIT_FAILURE);
          }
          break;

        /*Caballos*/
        default:
          srand(getpid());
          if(caballo(i-3, longitud, pipesIda[i-3], pipesVuelta[i-3]) == ERROR){
            perror("Error en el caballo");
            free(sem);
            shmdt((char*)carrera_info);
            msgctl (id_cola, IPC_RMID, (struct msqid_ds *)NULL);
            for(i = 0; i <caballos; i++){
              free(pipesIda[i]);
              free(pipesVuelta[i]);
            }
            free(pipesIda);
            free(pipesVuelta);
            free(childpid);
            exit(EXIT_FAILURE);
          }
          break;
      }
      /*Liberamos y salimos*/
      free(sem);
      shmdt((char*)carrera_info);
      for(i = 0; i <caballos; i++){
        free(pipesIda[i]);
        free(pipesVuelta[i]);
      }
      free(pipesIda);
      free(pipesVuelta);
      free(childpid);
      exit(EXIT_SUCCESS);
    }
  }

  /*Liberamos y eliminamos todo y salimos*/
  for(i = 0; i < caballos+3; i++) wait(NULL);
  for(i = 0; i <caballos; i++){
    free(pipesIda[i]);
    free(pipesVuelta[i]);
  }
  free(pipesIda);
  free(pipesVuelta);
  free(sem);
  free(childpid);
  Borrar_Semaforo(semid);
  shmdt((char*)carrera_info);
  shmctl (id, IPC_RMID, (struct shmid_ds *)NULL);
  msgctl (id_cola, IPC_RMID, (struct msqid_ds *)NULL);
  exit(EXIT_SUCCESS);
}
