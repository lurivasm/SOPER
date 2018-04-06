/**
*@brief Ejercicio4
*@author Lucia Rivas Molina
*@author Daniel Santo-Tomas Lopez
*@date 31/03/2018
*@file ejercicio4.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

/**
*@brief Numero de hijos a crear
*/
#define TAM 5

/**
*@brief Funcion capturadora de la señal
*/
void captura();

/**
*@brief Main del ejercicio 4
*@return EXIT_SUCCESS o EXIT_FAILURE
*/
int main(){
    int  i, j, status;
    pid_t hijo[5];
    /*Hacemos la captura de la señal*/
    signal(SIGUSR1, captura);

    for(i = 0; i < TAM; i++){
        hijo[i] = fork();

        /*En caso de error*/
        if(hijo[i] < 0){
        printf("ERROR\n");
        exit(EXIT_FAILURE);
        }

        /*En el padre*/
        else if(hijo[i] > 0 ){
            /*Si estamos en la primera iteración no esperamos a ningún hijo anterior*/
            if(i != 0){
            waitpid(hijo[i-1], &status, WUNTRACED | WCONTINUED);
            }
            pause();
            /*
            *Le mandamos una señal al último hijo y le esperamos
            * pues no hay siguiente hijo que se la mande
            */
            if(i == TAM-1){
              kill(hijo[i], SIGTERM);
              waitpid(hijo[i], &status, WUNTRACED | WCONTINUED);
              printf("Programa terminado\n");
              break;
            }
            continue;
        }

        /*En el hijo*/
        else if(hijo[i] == 0){
            /*En la primera iteración el hijo no elimina ningún otro proceso hijo*/
            if(i != 0){
              kill(hijo[i-1], SIGTERM);
            }
            for(j = 0 ; j < 10 ; j++){
                printf("Soy %d y estoy trabajando\n",getpid());
                fflush(stdout);
                sleep(1);
            }
            /*Le manda una señal al padre*/
            kill(getppid(),SIGUSR1);
            while(1){
                printf("Soy %d y estoy trabajando\n",getpid());
                sleep(1);
            }
            printf("Soy %d y voy a acabar\n",getpid());
            exit(EXIT_SUCCESS);
        }

    }
    /*Salimos*/
    exit(EXIT_SUCCESS);
}

/*Funcion capturadora de la señal*/
void captura(){
  return;
}
