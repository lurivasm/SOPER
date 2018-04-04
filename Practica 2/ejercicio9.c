/**
*@brief Ejercicio9
*@author Lucia Rivas Molina
*@author Daniel Santo-Tomas Lopez
*@date 31/03/2018
*@file ejercicio9.c
*/

#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include "semaforos.h"

/**
*@brief numero de cajas del supermercado
*/
#define CAJAS 4
/**
*@brief numero de semaforos necesarios
*Necesitamos un semaforo para el archivo de cada
* caja y uno más para el archivo info.txt que guarda
* la señal mandada y la caja que la envía
*/
#define SEMAFOROS CAJAS+1
/**
*@brief numero de corbos a clientes que realiza cada caja
*/
#define OPERAC 10
/**
*@brief establece un numero aleatorio entre 0 y 300
* para la escritura de los archivos clientesCaja.txt
*/
#define ALEAT aleat_num(0, 300)
/**
*@brief establece un numero aleatorio entre 0 y 5
* para los segundos del sleep de cada caja
*/
#define SECS aleat_num(0,5)
/**
*@brief key para los semaforos
*/
#define SEMKEY 75798

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
*@brief Lee el numero guardado en el fichero archivo y lo
* devuelve. En caso de error devuelve ERROR
*@param archivo nombre del fichero a leer
*@return el numero leido o ERROR
*/
int leer(char *archivo);

/**
*@brief Escribe el numero cantidad en el fichero archivo y
* devuelve, en caso de error ERROR y si sale bien OK
*@param archivo nombre del fichero a escribir
*@param cantidad a escribir
*@return OK o ERROR
*/
int escribir(char *archivo, int cantidad);

/**
*@brief Funcion capturadora de la señal. Si la señal capturada
* es SIGUSR1 el padre retira 900 euros. Si la señal es SIGUSR2
* el padre retira todo el dinero pues el hijo ha terminado
*@param signal señal que captura
*@return void
*/
void captura(int signal);


/**
*@brief Main del ejercicio 9
*@return EXIT_FAILURE o EXIT_SUCCESS
*/
int main(void) {
  int dinero = 0, total = 0, aleat, semid, status, i, j, cont = 0, sennal, hijo;
  char clientesCaja[CAJAS][50]; /*Nombres de los ficheros donde se almacena el pago de clientes*/
  char cajaTotal[CAJAS][50];    /*Nombres de los ficheros donde se almacena el total de la caja*/
  char info[9] = "info.txt";    /*Fichero donde se almacena la informacion que envia la caja*/
  unsigned short *sem;
  FILE *caja[CAJAS], *clientes[CAJAS];
  FILE *fich;
  pid_t pid[CAJAS];

  time(NULL);

  /*Creamos el nombre de los ficheros y generamos los números aleatorios*/
    for(i = 0; i < CAJAS; i++){
      /*Creamos el nombre de los ficheros que vamos a usar*/
      sprintf(clientesCaja[i], "clientesCaja%d.txt", i+1);
      sprintf(cajaTotal[i], "cajaTotal%d.txt", i+1);
      /*Abrimos el de los clientes y escribimos 50 numeros aleatorios*/
      clientes[i] = fopen(clientesCaja[i], "w");
      caja[i] = fopen(cajaTotal[i], "w");
      if (!clientes[i] || !caja[i]){
        return ERROR;
      }
      fprintf(caja[i], "%d\n", 0);
      for(j = 0; j < OPERAC; j++){
        aleat = ALEAT;
        fprintf(clientes[i], "%d\n", aleat);
      }
      if (fclose(clientes[i]) != 0 || fclose(caja[i]) != 0) {
        perror("Error en el cierre de ficheros");
        exit(EXIT_FAILURE);
      }
    }



  /*Creamos las señales*/
  if (signal(SIGUSR1, captura) == SIG_ERR){
    perror("Error en las señales");
    exit(EXIT_FAILURE);
  }
  if (signal(SIGUSR2, captura) == SIG_ERR){
    perror("Error en las señales");
    exit(EXIT_FAILURE);
  }

  /*
  *Creamos e inicializamos los semáforos, uno para cada caja
  * y el último para el fichero info.txt
  */
  if (Crear_Semaforo(SEMKEY, SEMAFOROS, &semid) == ERROR){
    perror("Error en el semaforo");
    exit(EXIT_FAILURE);
  }
  sem = (unsigned short*)malloc(sizeof(short)*(SEMAFOROS));
  if(!sem){
    Borrar_Semaforo(semid);
    perror("Error en el semaforo");
    exit(EXIT_FAILURE);
  }
  for(i = 0; i < SEMAFOROS; i++){
    sem[i] = 1;
  }
  if (Inicializar_Semaforo(semid, sem) == ERROR){
    Borrar_Semaforo(semid);
    free(sem);
    perror("Error en el semaforo");
    exit(EXIT_FAILURE);
  }

  printf("Abriendo tienda...\n");
  printf("Hay %d cajas operativas\nEsperando respuesta...\n\n", CAJAS);

  /*El proceso padre genera las cajas hijas y cada una realiza su operación*/
  for(i = 0; i < CAJAS; i++){
    /*En caso de error en el fork*/
    if((pid[i] = fork()) < 0){
      Borrar_Semaforo(semid);
      free(sem);
      perror("Error en el fork");
      exit(EXIT_FAILURE);
    }

    /*En caso del padre*/
    else if(pid[i] > 0){
      /*Primero genera todas las cajas con el continue*/
      if (i < CAJAS-1) continue;
      /*Una vez estás todas las cajas realiza un pause()*/
      while(cont < CAJAS){
        pause();
        /*Lee de info.txt que señal recibe y que hijo la manda*/
        fich = fopen(info, "r");
        fscanf(fich, "%d %d", &sennal, &hijo);
        fclose(fich);
        switch (sennal) {
          /*En caso de que sea SIGUSR1 restamos 900 euros al dinero*/
          case SIGUSR1:
            printf("CAJA %d : supervisor retirando 900 euros\n\n", hijo+1);
            Down_Semaforo(semid, hijo, SEM_UNDO);
            /*Lee el total de la caja del hijo*/
            dinero = leer(cajaTotal[hijo]);
            if(total == ERROR){
              perror("Error en la lectura de ficheros");
              Borrar_Semaforo(semid);
              free(sem);
              exit(EXIT_FAILURE);
            }
            /*Le resta 900 euros al dinero de la caja del hijo y los suma al total*/
            dinero -= 900;
            total += 900;
            /*Volver a escribir el dinero en cajaTotal.txt*/
            if(escribir(cajaTotal[hijo], dinero) == ERROR){
              perror("Error en la escritura de ficheros");
              Borrar_Semaforo(semid);
              free(sem);
              exit(EXIT_FAILURE);
            }
            /*Up del semaforo del hijo y del de info.txt para que otro hijo le avise*/
            Up_Semaforo(semid, CAJAS, SEM_UNDO);
            Up_Semaforo(semid, hijo, SEM_UNDO);

            break;
          /*En caso de que sea SIGUSR2 sacamos todo el dinero del fichero*/
          case SIGUSR2:
            printf("CAJA %d : supervisor retirando dinero\n\n", hijo+1);
            Down_Semaforo(semid, hijo, SEM_UNDO);
            caja[hijo] = fopen(cajaTotal[hijo], "r");
            fscanf(caja[hijo], "%d\n", &dinero);
            total += dinero;
            fclose(caja[hijo]);
            Up_Semaforo(semid, hijo, SEM_UNDO);
            Up_Semaforo(semid, CAJAS, SEM_UNDO);
            cont++;
            break;
        }
      }
      printf("El total ganado hoy es %d\n", total);
      /*Esperamos a los hijos*/
      for(i = 0; i < CAJAS; i++){
        waitpid(pid[i], &status, WUNTRACED | WCONTINUED);
      }
    }

    /*Los hijos leen su correspondiente clientesCaja y van sumando el dinero en cajaTotal*/
    else{
      clientes[i] = fopen(clientesCaja[i], "r");

      while(!feof(clientes[i])){
        printf("\t Operando caja %d\n", i+1);
        Down_Semaforo(semid, i, SEM_UNDO);
        /*Cobra al cliente leyendo de clientesTotal.txt y lo suma al total*/
        fscanf(clientes[i], "%d\n", &dinero);
        total = leer(cajaTotal[i]);
        if(total == ERROR){
          perror("Error en la lectura de ficheros");
          Borrar_Semaforo(semid);
          free(sem);
          exit(EXIT_FAILURE);
        }
        total += dinero;

        /*Volver a escribit el total en cajaTotal.txt*/
        if(escribir(cajaTotal[i], total) == ERROR){
          perror("Error en la escritura de ficheros");
          Borrar_Semaforo(semid);
          free(sem);
          exit(EXIT_FAILURE);
        }

        /*Si el total de dinero supera los 1000 mandamos una señal al padre*/
        if (1000 <= total){
          Down_Semaforo(semid, CAJAS, SEM_UNDO);
          fich = fopen(info, "w");
          fprintf(fich, "%d %d\n", SIGUSR1, i);
          fclose(fich);
          kill(getppid(), SIGUSR1);
        }
        /*Up del semaforo de cajaTotal.txt para el padre*/
        Up_Semaforo(semid, i, SEM_UNDO);
        sleep(SECS);
      }

      /*Cerramos el fichero de clientes y avisamos al padre de que hemos terminado*/
      fclose(clientes[i]);
      Down_Semaforo(semid, CAJAS, SEM_UNDO);
      /*Escribimos en info.txt la señal y el hijo*/
      fich = fopen(info, "w");
      fprintf(fich, "%d %d\n", SIGUSR2, i);
      fclose(fich);
      kill(getppid(), SIGUSR2);
      exit(EXIT_SUCCESS);
    }
  }

  /*Borramos los semaforos y liberamos memoria*/
  Borrar_Semaforo(semid);
  free(sem);
  exit(EXIT_SUCCESS);
}


/*FUNCIONES AUXILIARES*/

int aleat_num(int inf, int sup){
  int aux;
  /*En caso de que sup sea menor que inf los permuta*/
  if( sup < inf ){
    aux = sup;
    sup = inf;
    inf = aux;
  }
  /*En caso de que alguno sea negativo los cambia a positivo*/
  if(sup < 0) sup = -sup;
  if(inf < 0) inf = -inf;
  return inf+(rand()%(sup-inf+1));
}


void captura(int sig){
  switch (sig) {
    /*SIGUSR1 avisa de que ha superado los 1000 euros*/
    case SIGUSR1:
      printf("Caja llena, informando al supervisor\n");
      break;
    /*SIGUSR2 avisa de que ha terminado la caja*/
    case SIGUSR2:
      printf("Caja terminada, informando al supervisor\n");
      break;
  }
  return;
}


int leer(char *archivo){
  FILE *f;
  int leido;
  if(!archivo) return ERROR;
  /*Abre el fichero archivo en modo lectura*/
  if(!(f = fopen(archivo, "r"))) return ERROR;
  /*Lee el valor, cierra y devuelve el valor leido*/
  fscanf(f, "%d\n", &leido);
  if(fclose(f) != 0) return ERROR;
  return leido;
}


int escribir(char *archivo, int cantidad){
  FILE *f;
  if(!archivo || cantidad < 0) return ERROR;
  /*Abre el fichero archivo en modo sobreescritura*/
  if(!(f = fopen(archivo, "w"))) return ERROR;
  /*Escribe el valor cantidad y cierra*/
  fprintf(f, "%d\n", cantidad);
  if(fclose(f) != 0) return ERROR;
  return OK;
}
