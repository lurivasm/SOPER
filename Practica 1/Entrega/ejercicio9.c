/**
*@brief Ejercicio9
*@author Lucia Rivas Molina <lucia.rivasmolina@estudiante.uam.es>
*@author Daniel Santo-Tomas Lopez <daniel.santo-tomas@estudiante.uam.es>
*@file ejercicio9.c
*@date 2018/03/07
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>
#define MAX_TAM 100


/**
*@brief Calcula el factorial del numero pasado como parametro
*@param un numero real
*@return factorial del numero
*/
double factorial(double a){
  if(a < 0) return -1;
  if(a == 0) return 1;
  return a*factorial(a-1);
}

/**
*@brief Calcula el coeficiente binomial de a sobre b
*@param numero real a
*@param numero real b
*@return a sobre b
*/
double combinatoria(double a, double b){
  if (a < b || a < 0 || b < 0 || (a == 0 && b != 0)) return 0;
  if(b == 0) return 1;
  return combinatoria(a-1, b-1) + combinatoria(a-1, b);
}

/**
*@brief Calcula la suma de dos valores absolutos
*@param primer operando
*@param segundo operando
*@return suma de los valores absolutos
*/
double sumAbs(double a, double b){
  return fabs(a) + fabs(b);
}


/**
*Programa que lanza cuatro hijos y cada uno realiza una operación matemática
*@brief Funcion main
*/
int main(void){
  pid_t childpid[4];                          /*Los pids correspondientes a los hijos*/
  double o1, o2, result;                      /*Operandos para cada fork*/
  char buffer[MAX_TAM], respuesta[MAX_TAM], pid[MAX_TAM]; /*Strings para las tuberías*/
  int i, status;
  int fdIda[4][2];      /*Tuberías de escritura del padre y lectura de los hijos*/
  int fdVuelta[4][2];      /*Tuberías de escritura de los hijos y lectura del padre*/


  /*Pedimos los datos*/
  printf("Introduce el primer operando: ");
  scanf("%lf", &o1);
  printf("\nIntroduce el segundo operando: ");
  scanf("%lf", &o2);
  sprintf(buffer, "%.3lf,%.3lf", o1, o2);

  for(i = 0; i < 4; i++){
    /*Creamos las tuberías de ida y vuelta*/
    if(pipe(fdIda[i]) == -1 || pipe(fdVuelta[i]) == -1){
      perror("Error en el pipe");
      exit(EXIT_FAILURE);
    }
    /*Creamos el hijo*/
    childpid[i] = fork();
    /*ERROR*/
    if(childpid[i] < -1) {
      perror("Error en el fork");
      exit(EXIT_FAILURE);
    }
    /*En el hijo*/
    else if(childpid[i] == 0){
      printf("Hijo %d recibiendo datos\n", i+1);
      /*Cerramos las tuberías de escritura y lectura respectivas*/
      close(fdIda[i][1]);
      close(fdVuelta[i][0]);

      /*Leemos los datos*/
      if (read(fdIda[i][0], pid, MAX_TAM) < 0 ){
        perror("Error en el read");
        exit(EXIT_FAILURE);
      }
      sscanf(pid, "%lf,%lf", &o1, &o2);

      /*Primer proceso calcula la potencia*/
      if(i == 0){
        result = pow(o1, o2);
        sprintf(respuesta, "%.3lf elevado a %.3lf = %.3lf", o1, o2, result);
      }
      /*Segundo proceso calcula factorial*/
      else if(i == 1){
        result = factorial(o1)/o2;
        sprintf(respuesta, "factorial(%.3lf) entre %.3lf = %.3lf", o1, o2, result);
      }
      /*Tercer proceso calcula combinatoria*/
      else if(i == 2){
        result = combinatoria(o1, o2);
        sprintf(respuesta, "%.3lf sobre %.3lf = %.3lf", o1, o2, result);
      }
      /*Cuarto proceso calcula la suma en valor absoluto*/
      else if(i == 3){
        result = sumAbs(o1, o2);
        sprintf(respuesta, "abs(%.3lf + %.3lf) = %.3lf", o1, o2, result);
      }

      if(write(fdVuelta[i][1], respuesta, MAX_TAM) < 0){
        perror("Error en el write");
        exit(EXIT_FAILURE);
      }
      exit(0);
    }

    /*En el padre*/
    else{
        /*Cerramos las tuberías de escritura y lectura respectivas*/
        close(fdIda[i][0]);
        close(fdVuelta[i][1]);

        /*Escribimos el mensaje*/
        if(write(fdIda[i][1], buffer, MAX_TAM) < 0){
          perror("Error en el write");
          exit(EXIT_FAILURE);
        }

        /*Recibimos la respuesta*/
        waitpid(childpid[i], &status, WUNTRACED | WCONTINUED);
        if(read(fdVuelta[i][0], respuesta, MAX_TAM) < 0){
          perror("Error en el read");
          exit(EXIT_FAILURE);
        }
        printf("\tHijo %d devuelve : %s \n", i+1, respuesta);
        continue;
      }
  }
  exit(EXIT_SUCCESS);
}
