/**
*@brief Ejercicio3
*@author Lucia Rivas Molina
*@author Daniel Santo-Tomas Lopez
*@date 31/03/2018
*@file ejercicio2.c
*/
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

/**
*@brief FUncion capturadora de la señal
*/
void captura (int sennal);

/**
*@brief Main del ejercicio
*/
int main (int argc, char *argv [], char *env []){
  if (signal (SIGINT, captura) == SIG_ERR){
    puts ("Error en la captura");
    exit (1);
  }
  while (1);
  exit (0);
}

/*FUNCIONES AUXILIARES*/
void captura (int sennal){
  printf ("Capturada la señal %d \n", sennal);
  fflush (NULL);
  return;
}
