/**
*@brief Ejercicio8
*@author Lucia Rivas Molina <lucia.rivasmolina@estudiante.uam.es>
*@author Daniel Santo-Tomas Lopez <daniel.santo-tomas@estudiante.uam.es>
*@file ejercicio8.c
*@date 2018/03/07
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_TAM 15
#define ERROR -1
#define OK 0

/**
*@brief Función que ejecuta el hijo
*@param tipo : tipo de exec que queremos (-l, -lp...)
*@param programa : nombre del programa a ejecutar(ls)
*@return ERROR en caso de error y OK si se ha ejecutado
*/
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
    free(path1);
    free(path2);
    return OK;
  }

  if(strcmp("-lp", tipo) == 0){
    execlp(programa, programa, NULL);
    free(path1);
    free(path2);
    return OK;
  }

  if(strcmp("-v", tipo) == 0){
    sprintf(path1, "/bin/%s", programa);
    sprintf(path2, "usr/bin/%s", programa);
    char* prog[] = {programa, NULL};
    execv(path1, prog);
    execv(path2, prog);
    free(path1);
    free(path2);
    return OK;
  }

  if(strcmp("-vp", tipo) == 0){
    char* prog[] = {programa, NULL};
    execvp(programa, prog);
    free(path1);
    free(path2);
    return OK;
  }
  /*En caso de que no se haya metido correctamente el valor de la función*/
  free(path1);
  free(path2);
  return ERROR;
}

/**
*Programa que ejecuta los programas pasados como parámetros
*@brief Funcion main
*@param programas a ejecutar
*@param exec que queremos ejecutar (-l -lp -v o -vp)
*/
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
      /*En caso de error en el fork*/
      if ((pid = fork()) < 0 ){
        printf("Error haciendo fork\n");
        exit(EXIT_FAILURE);

      }
      /*Si estamos en el hijo ejecutamos el programa*/
      else if (pid == 0){
        if(ejecutar(argv[argc -1], argv[i]) == ERROR){
          printf("Error en exec");
        }
      }
      /*Si estamos en el padre esperamos al hijo*/
      else{
        waitpid(pid, &status, WUNTRACED | WCONTINUED);
      }

  }
  wait(NULL);
  exit(EXIT_SUCCESS);
}
