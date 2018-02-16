#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_TAM 15
#define ERROR -1
#define OK 0

int ejecutar(char *tipo, char *programa){
  if(!tipo || !programa){
    return ERROR;
  }

    /*Reservamos memoria para las cadenas*/
    char *path1 = (char*)malloc(sizeof(char)*MAX_TAM);
    if(!path1){
      printf("Error al reservar memoria\n");
      return ERROR;
    }
    char *path2 = (char*)malloc(sizeof(char)*MAX_TAM);
    if(!path2){
      printf("Error al reservar memoria\n");
      free(path1);
      return ERROR;
    }

  /*Nos metemos en el exec indicado*/
  if(strcmp("-l", tipo) == 0){
    sprintf(path1, "/bin/%s", programa);
    sprintf(path2, "usr/bin/%s", programa);
    execl(path1, programa, NULL);
    execl(path2, programa, NULL);
    return OK;
  }

  if(strcmp("-lp", tipo) == 0){
    execlp(programa, programa, NULL);
    return OK;
  }

  if(strcmp("-v", tipo) == 0){
    sprintf(path1, "/bin/%s", programa);
    sprintf(path2, "usr/bin/%s", programa);
    char* prog[] = {programa, NULL};
    execv(path1, prog);
    execv(path2, prog);
    return OK;
  }

  if(strcmp("-vp", tipo) == 0){
    char* prog[] = {programa, NULL};
    execvp(programa, prog);
    return OK;
  }
  return ERROR;
}



int main(int argc, char **argv){
  int pid;
  int i;
  int status;

  /*Comprobamos los parámetros de entrada*/
  if(argc < 3){
    printf("Error en los parámetros de entrada\n");
    printf("Introducir: ./Ejercicio8 <programas> -l o -lp o -v o -vp\n");
    exit(EXIT_FAILURE);
  }

  for (i = 1; i <= argc -1; i++){
      if ((pid = fork()) < 0 ){
        printf("Error haciendo fork\n");
        exit(EXIT_FAILURE);

      }
      /*Si estamos en el hijo*/
      else if (pid == 0){
        if(ejecutar(argv[argc -1], argv[i]) == ERROR){
          printf("Error en exec");
        }
      }
      /*Si estamos en el padre*/
      else{
        waitpid(pid, &status, WUNTRACED | WCONTINUED);
      }

  }
  wait();
  exit(EXIT_SUCCESS);
}
