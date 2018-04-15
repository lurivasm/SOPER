/**
*@brief Cadena de montaje Ejercicio5
*@author Lucia Rivas Molina
*@author Daniel Santo-Tomas Lopez
*@date 14/04/2018
*@file cadena_montaje.c
*/
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
*@brief filekey para la clave de la memoria compartida
*/
#define FILEKEY "/bin/ls"
/**
*@brief key para la memoria compartida
*/
#define KEY 33
/**
*@brief retorno en caso de error
*/
#define ERROR -1
/**
*@brief retorno en caso de OK
*/
#define OK 0
/**
* Compuesta por el tipo, la info y un int que indica si ha acabado
*@brief Estructura de los mensajes
*/
typedef struct {
  long mtype;
  int acabado;
  char info[16];
} Mensaje;
/**
*@brief Funcion que solo cambia las letras por su siguiente
*@param cadena : string a modificar
*@return la cadena modificada o ERROR en caso de ERROR
*/
char *cambiar(char *cadena);
/**
* El proceso A lee el fichero y manda de 16b en 16 b mensajes a B
*@brief Funcion que realiza las acciones del proceso A
*@param id_cola : el id de la cola
*@param entrada : path del fichero a leer
*@return OK o ERROR
*/
int procesoA(int id_cola, char* entrada);
/**
*Lee los mensajes de A, los transforma y se los manda a C
*@brief Funcion que realiza las acciones del proceso B
*@param id_cola : el id de la cola
*@return OK o ERROR
*/
int procesoB(int id_cola);
/**
*Lee los mensajes de B y los escribe en un fichero
*@brief Funcion que realiza las acciones del proceso C
*@param id_cola : el id de la cola
*@param salida : path del fichero a escribir
*@return OK o ERROR
*/
int procesoC(int id_cola, char* salida);

/**
*@brief Main del ejercicio 5 cadena de montaje
*@return EXIT_FAILURE o EXIT_SUCCESS
*/
int main(int argc, char **argv){
  pid_t pid;
  int key, id_cola, i;

  /*Comprobamos los parámetros de entrada*/
  if(argc < 3){
    printf("No hay suficientes parámetros de entrada\n");
    printf("Intenta : ./cadena_montaje entrada.txt salida.txt\n");
    exit(EXIT_FAILURE);
  }

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

  /*Creamos los hijos*/
  for(i = 0; i < 2; i++){
    pid = fork();
    /*En caso de error*/
    if(pid < 0){
      perror("Error en el fork");
      exit(EXIT_FAILURE);
    }

    /*En el padre, que es el proceso C*/
    else if(pid > 0){
      if(i == 0) continue;
      if(procesoC(id_cola, argv[2]) == -1){
        perror("Error proceso C");
        exit(EXIT_FAILURE);
      }
      /*Eliminamos los mensajes y esperamos a los hijos*/
      msgctl (id_cola, IPC_RMID, (struct msqid_ds *)NULL);
      wait(NULL);
      wait(NULL);
    }

    /*En el hijo*/
    else{
      switch (i) {
        /*El proceso A*/
        case 0:
          if(procesoA(id_cola, argv[1]) == -1){
            perror("Error proceso A");
            exit(EXIT_FAILURE);
          }
          exit(EXIT_SUCCESS);
        /*El proceso B*/
        case 1:
          if(procesoB(id_cola) == -1){
            perror("Error proceso B");
            exit(EXIT_FAILURE);
          }
          exit(EXIT_SUCCESS);
      }
    }
  }
  /*Acaba el ejercicio y sale*/
  printf("Trabajo acabado con éxito!\n");
  exit(EXIT_SUCCESS);
}


/*Funcion correspondiente al proceso A*/
int procesoA(int id_cola, char* entrada){
  FILE *fich;
  Mensaje mensaje;

  if(!entrada) return ERROR;

  /*Inicializamos acababado a 0 y el tipo a 1 para mandarselo a B*/
  mensaje.acabado = 0;
  mensaje.mtype = 1;
  fich = fopen(entrada, "r");
  if(!fich) return ERROR;
  /*Leemos el fichero hasta un \n*/
  fgets(mensaje.info, 16, fich);
  while(!feof(fich)){
    if(msgsnd(id_cola, (struct msgbuf *)&mensaje, sizeof(Mensaje) - sizeof(long), 0) < 0){
      fclose(fich);
      return ERROR;
    }
    fgets(mensaje.info, 16, fich);
  }
  /*Cerramos, cambiamos acabado a 1 para indicar que A termina y enviamos el mensaje final*/
  fclose(fich);
  mensaje.acabado = 1;
  if(msgsnd(id_cola, (struct msgbuf *)&mensaje, sizeof(Mensaje) - sizeof(long), 0) < 0){
    fclose(fich);
    return ERROR;
  }
  printf("Proceso A acabado\n");
  return OK;
}

/*Funcion correspondiente al proceso B*/
int procesoB(int id_cola){
  Mensaje mensaje;
  int rcv, acabado = 0;
  char *aux;

  /*Recibimos el primer mensaje y comprobamos el tamaño y si hay errores*/
  rcv = msgrcv(id_cola, (struct msgbuf *)&mensaje, sizeof(Mensaje) - sizeof(long), 1, MSG_NOERROR);
  if(rcv < 0) return ERROR;

  while(acabado == 0){
    /*Cambiamos el mensaje a tipo 2 para que lo reciba C*/
    mensaje.mtype = 2;
    /*Cambiamos el mensaje y lo enviamos*/
    aux = cambiar(mensaje.info);
    if(!aux) return ERROR;
    strcpy(mensaje.info, aux);
    if(msgsnd(id_cola, (struct msgbuf *)&mensaje, sizeof(Mensaje) - sizeof(long), 0) < 0) return ERROR;

    /*Recibimos el mensaje y comprobamos el tamaño y si hay errores*/
    rcv = msgrcv(id_cola, (struct msgbuf *)&mensaje, sizeof(Mensaje) - sizeof(long), 1, MSG_NOERROR);
    if(rcv < 0) return ERROR;
    acabado = mensaje.acabado;
  }
  /*Mandamos el mensaje final y salimos*/
  mensaje.mtype = 2;
  if(msgsnd(id_cola, (struct msgbuf *)&mensaje, sizeof(Mensaje) - sizeof(long), 0) < 0) return ERROR;
  printf("Proceso B acabado\n");
  return OK;
}

/*Funcion correspondiente al proceso C*/
int procesoC(int id_cola, char* salida){
  Mensaje mensaje;
  FILE *fich;
  int rcv, acabado = 0;
  if(!salida) return ERROR;

  /*Abrimos el fichero*/
  fich = fopen(salida, "w");
  if(!fich) return ERROR;

  /*Recibimos el mensaje de tipo 2 y comprobamos el tamaño y si hay errores*/
  rcv = msgrcv(id_cola, (struct msgbuf *)&mensaje, sizeof(Mensaje) - sizeof(long), 2, MSG_NOERROR);
  if(rcv < 0) return ERROR;
  acabado = mensaje.acabado;
  /*Mientras que el proceso B no haya terminado C no termina*/
  while(acabado == 0){
    /*Escribimos el mensaje en el fichero salida*/
    fprintf(fich, "%s", mensaje.info);
    /*Recibimos el mensaje de tipo 2 y comprobamos el tamaño y si hay errores*/
    rcv = msgrcv(id_cola, (struct msgbuf *)&mensaje, sizeof(Mensaje) - sizeof(long), 2, MSG_NOERROR);
    if(rcv < 0) return ERROR;
    acabado = mensaje.acabado;
  }
  /*Cerramos y salimos*/
  fclose(fich);
  printf("Proceso C acabado\n");
  return OK;
}

/*Funcion que cambia las letras del abecedario por su siguiente*/
char *cambiar(char *cadena){
  int i;
  if(!cadena) return NULL;
  for(i = 0; i < 16; i++){
    if('a'<= cadena[i] && cadena[i] < 'z') cadena[i]++;
    else if('A'<= cadena[i] && cadena[i] < 'Z') cadena[i]++;
    else if(cadena[i] == 'z') cadena[i] = 'a';
    else if(cadena[i] == 'Z') cadena[i] = 'A';
  }
  return cadena;
}
