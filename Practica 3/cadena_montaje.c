#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

#define FILEKEY "/bin/ls"
#define KEY 33


typedef struct {
  long mtype;
  int acabado;
  char info[17];
} Mensaje;

int main(int argc, char **argv){
  pid_t pid1, pid2;
  char *entrada, *salida;
  int key, id_cola;

  /*Creamos la cola de mensajes*/
  key = ftok(FILEKEY, KEY);
  if(key == -1){
    perror("Error de clave");
    exit(EXIT_FAILURE);
  }
  id_cola = msgget(key, IPC_CREAT | 0660);
  if(id_cola == -1){
    perror("Error de clave");
    exit(EXIT_FAILURE);
  }

  /*Comprobamos los parámetros de entrada*/
  if(argc < 3){
    printf("No hay suficientes parámetros de entrada\n");
    printf("Intenta : ./cadena_montaje entrada.txt salida.txt\n");
    exit(EXIT_FAILURE);
  }

  for(i = 0; i < 2; i++){
    pid = fork();

    if(pid < 0){
      perror("Error en el fork");
      exit(EXIT_FAILURE);
    }

    /*En el padre*/
    else if(pid > 0){
      if(i == 0) continue;
      if(procesoC(id_cola, argv[2]) == -1){
        perror("Error proceso C");
        exit(EXIT_FAILURE);
      }
      wait(NULL);
      wait(NULL);
    }

    /*En el hijo*/
    else{
      switch (i) {
        case 0:
          if(procesoA() == -1){
            perror("Error proceso A");
            exit(EXIT_FAILURE);
          }
          exit(EXIT_SUCCESS);

        case 1:
          if(procesoB() == -1){
            perror("Error proceso B");
            exit(EXIT_FAILURE);
          }
          exit(EXIT_SUCCESS);
      }

    }
  }
  exit(EXIT_SUCCESS);
}

int procesoA(int id_cola, char* entrada){
  FILE *fich;
  Mensaje mensaje;

  fich = fopen(entrada, "r");
  while(!feof(fich)){
    fgets()
  }

}

int procesoB(){

}

int procesoC(){

}
