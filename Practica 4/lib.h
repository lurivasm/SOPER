/**
*@brief Libreria para las funciones de la carrera entre las que se encuentran
* las funciones de cada proceso hijo y padre, hilos y auxiliares
*@author Lucia Rivas Molina
*@author Daniel Santo-Tomas Lopez
*@date 9/05/2018
*@file lib.h
*/
#define  _GNU_SOURCE
#ifndef LIB_H
#define LIB_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <syslog.h>

#include "semaforos.h"

/**
*@brief maximo numero de caballos
*/
#define MAX_CABALLOS 10
/**
*@brief maximo numero de apostadores
*/
#define MAX_APOSTADORES 100
/**
*@brief file para la clave
*/
#define FILEKEY "/bin/cat"
/**
*@brief key para los semaforos y la memoria compartida
*/
#define KEY 1300
/**
*@brief Estructura pasada como argumento a los hilos
*  num_ventana : numero de ventanilla que es
*  id_mem : id de la memoria compartida
*/
typedef struct{
  int num_ventana;      /*id de la ventana*/
  int id_cola;         /*id de la cola de mensajes*/
  int id_mem;          /*id de la memoria compartida*/
} Hilo_args;
/**
*@brief Mensaje de cada apostador al gestor de apuestas
*/
typedef struct{
  long mtype;         /*Tipo de mensaje (por defecto será 1)*/
  char nombre[16];    /*Nombre del apostador*/
  double apuesta;     /*Apuesta que realiza*/
  int caballo;        /*Caballo por el que apuesta*/
  int ventanilla;     /*Numero de ventanilla en la que apuesta*/
  double cotizacion;  /*Cotizacion del caballo apostado antes de apostar*/
  double pago;        /*Pago al apostador en caso de que gane*/
} Msg_apuesta;
/**
*@brief Memoria compartida entre principal, gestor y monitor
*/
typedef struct{
  int tiradas[MAX_CABALLOS];             /*Tiradas de los caballos*/
  int recorridos[MAX_CABALLOS];          /*Recorridos de los caballos*/
  int acabado;                           /*Flag para controlar que la carrera ha terminado*/
  double cotizacion[MAX_CABALLOS];       /*cotizacion de los caballos*/
  int total_apuestas;                    /*Total apostado a todos los caballos*/
  int total_caballo[MAX_CABALLOS];       /*Total apostado a cada caballo*/
  int cont;                              /*Contador de cuantas apuestas se realizan*/
  Msg_apuesta mensaje[MAX_APOSTADORES];  /*Mensajes de apuestas*/
} Carrera_info;

/**
*@brief Funcion del proceso padre del programa, encargado de realizar
* la comunicacion entre procesos y de asignarle tiradas a los caballos
*@param mask : mascara de señales permitidas
*@param pipeIda : tuberias de escritura con los caballos
*@param pipeVuelta : tuberias de lectura con los caballos
*@param caballos : numero de caballos
*@param longitud : longitud de la carrera
*@param semid : id de los semaforos
*@param info : estructura con la informacion de la carrera
*@param chilpid : id de los procesos hijos
*@return OK o ERROR
*/
int principal(sigset_t mask, int **pipeIda, int **pipeVuelta, int caballos, int longitud, int semid, Carrera_info *info, pid_t *childpid);
/**
*@brief Funcion que realiza el trabajo de los caballos mandando y
* recibiendo posiciones y recorridos con el padre (principal)
*@param num : numero de caballo que es
*@param pipeIda : tuberias de escritura con los caballos
*@param pipeVuelta : tuberias de lectura con los caballos
*@param longitud : longitud de la carrera
*@return OK o ERROR
*/
int caballo(int num, int longitud, int *pipeIda, int *pipeVuelta);
/**
*@brief Funcion encargada de imprimir por pantalla la informacion
*@param mask : mascara de señales permitidas
*@param caballos : numero de caballos
*@param longitud : longitud de la carrera
*@param semid : id de los semaforos
*@param info : estructura con la informacion de la carrera
*@param dinero : dinero maximo por apostador introducido por teclado
*@return OK o ERROR
*/
int monitor(sigset_t mask, int caballos, int longitud, int semid, Carrera_info *info, int dinero);
/**
*@brief Funcion encargada de crear apostadores y mandar mensajes a las ventanillas
*@param dinero : dinero maximo por apostador introducido por teclado
*@param caballos : numero de caballos
*@param apostadores : numero de apostadores introducido por teclado
*@param id_cola : id de la cola de mensajes
*@return OK o ERROR
*/
int apostador(int dinero, int caballos, int apostadores, int id_cola);
/**
*@brief Funcion encargada de inicializar las variables de la memoria
* compartida relacionadas con el dinero y de crear las ventanillas
*@param mask : mascara de señales permitidas
*@param id_cola : id de la cola de mensajes
*@param caballos : numero de caballos
*@param apostadores : numero de apostadores introducido por teclado
*@param ventanillas : numero de ventanillas introducido por teclado
*@param id : id de la cola de mensajes
*@return OK o ERROR
*/
int gestor(sigset_t mask, int id_cola, int caballos, int apostadores, int ventanillas, int id);
/**
*@brief Funcion de los hilos encargada de recibir las apuestas y
* agregarlas a la memoria compartida
*@param arg : estructura que contiene el id de la cola y el numero de ventanilla
*@return pthread_exit
*/
void *ventanilla(Hilo_args *arg);
/**
*@brief Funcion capturadora de las señales
*@param : SIGUSR1 para avisar al padre de que comienza la carrera
*         SIGUSR2 para avisar al gestor y al apostador de que acaben
*         SIGINT para acabar el programa con CTRL+C y borrar memoria
*/
void captura(int sennal);
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
* Se utiliza para los 10 apostadores con mas ganancias
*@brief Funcion que ordena la tabla ganadores en funcion de la tabla pago
*@param ganadores : tabla de los apostadores en orden de llegada
*@param pago : tabla del dinero de los apostadores en orden de llegada
*@param ip : primera iteracion
*@param iu : ultima iteracion
*@return numero de iteraciones y ERROR en caso de error
*/
int BubbleSort(int* ganadores, double *pago, int ip, int iu);

#endif
