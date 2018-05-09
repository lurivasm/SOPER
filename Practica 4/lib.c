/**
*@brief Libreria para las funciones de la carrera entre las que se encuentran
* las funciones de cada proceso hijo y padre, hilos y auxiliares
*@author Lucia Rivas Molina
*@author Daniel Santo-Tomas Lopez
*@date 9/05/2018
*@file lib.c
*/
#include "lib.h"

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
int principal(sigset_t mask, int **pipeIda, int **pipeVuelta, int caballos, int longitud, int semid, Carrera_info *info, pid_t *childpid){
  int i, ultimo = 0, primero = 0, acabado = 0;
  int posiciones[caballos], recorridos[caballos], tiradas[caballos];

  /*Control de errores*/
  if(!pipeIda || !pipeVuelta || !info || !childpid) return ERROR;
  if(!info->tiradas || !info->recorridos) return ERROR;
  /*Inicializamos los pipes, las posiciones y los recorridos*/
  for(i = 0; i < caballos; i++){
      close(pipeIda[i][1]);
      close(pipeVuelta[i][0]);
      posiciones[i] = 2;
      recorridos[i] = 0;
  }

  /*Hasta comenzar la carrera*/
  sigsuspend(&mask);
  syslog(LOG_INFO, "Proceso principal mandando tiradas");
  /*Matamos a los apostadores y al gestor de apuestas*/
  kill(childpid[2], SIGUSR2);
  kill(childpid[0], SIGUSR1);

  while(acabado != 1){
    /*Enviamos la posicion de cada caballo*/
    for(i = 0; i < caballos; i++) {
      if(recorridos[i] <= recorridos[ultimo] && i != 0) posiciones[ultimo] = 0;
      else if(recorridos[i] >= recorridos[primero] && i != 0) posiciones[primero] = 1;
      write(pipeVuelta[i][1], &posiciones[i], sizeof(int));
    }
    ultimo = 0;
    primero = 0;

    /*Leemos la tirada y recalculamos*/
    if(Down_Semaforo(semid, 1, SEM_UNDO) == ERROR) return ERROR;

    for(i = 0; i < caballos; i++) {
      read(pipeIda[i][0], &tiradas[i], sizeof(int));
      recorridos[i] += tiradas[i];
      if(recorridos[i] <= recorridos[ultimo]) ultimo = i;
      else if(recorridos[i] >= recorridos[primero]) primero = i;
      posiciones[i] = 2;
      info->recorridos[i] = recorridos[i];
      info->tiradas[i] = tiradas[i];

      /*Si un caballo alcanza la meta se acaba la carrera*/
      if(longitud <= recorridos[i]){
        acabado = 1;
        info->recorridos[i] = longitud;
      }
    }
    info->acabado = acabado;
    /*Up del semaforo 0 para que el monitor imprima*/
    if(Up_Semaforo(semid, 0, SEM_UNDO) == ERROR) return ERROR;
  }
  /*Avisamos a los caballos para que acaben*/
  acabado = -1;
  syslog(LOG_INFO, "La carrera ha terminado");
  for(i = 0; i < caballos; i++) write(pipeVuelta[i][1], &acabado, sizeof(int));

  return OK;
}

/**
*@brief Funcion que realiza el trabajo de los caballos mandando y
* recibiendo posiciones y recorridos con el padre (principal)
*@param num : numero de caballo que es
*@param pipeIda : tuberias de escritura con los caballos
*@param pipeVuelta : tuberias de lectura con los caballos
*@param longitud : longitud de la carrera
*@return OK o ERROR
*/
int caballo(int num, int longitud, int *pipeIda, int *pipeVuelta){
  int recorrido = 0, posicion = 0, tirada = 0;
  close(pipeIda[0]);
  close(pipeVuelta[1]);
  syslog(LOG_INFO, "Caballo %d leyendo tiradas", num+1);
  while(1){
    /*Recibimos la posicion en la que estamos*/
    read(pipeVuelta[0], &posicion, sizeof(int));
    /*Si recibimos -1 significa que debemos acabar*/
    if(posicion == -1) break;

    /*Si somos ultimos*/
    else if (posicion == 0){
      tirada = aleat_num(1,6);
      recorrido += tirada;
      tirada += aleat_num(1,6);
    }
    /*Si somos primeros*/
    else if (tirada == 1) tirada = aleat_num(1,7);
    else tirada = aleat_num(1,6);
    recorrido += tirada;

    /*Le enviamos la tirada al padre*/
    write(pipeIda[1], &tirada, sizeof(int));
  }
  syslog(LOG_INFO, "Caballo %d finalizando", num+1);
  return OK;
}

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
int monitor(sigset_t mask, int caballos, int longitud, int semid, Carrera_info *info, int dinero){
  int i, acabado = 0, cont = 10;
  int *ganadores;
  double *ganancias;
  /*Control de errores*/
  if(!info || dinero < 0 || caballos < 0 || longitud < 0) return ERROR;
  /*Contamos 30 segundos y avisamos al padre para comenzar la carrera*/
  for(i = 3; i > 0; i--){
    printf("Quedan %d segundos\n", i*10);
    alarm(10);
    sigsuspend(&mask);
  }
  kill(getppid(), SIGUSR1);
  sleep(3);
  /*Imprimimos las corizaciones*/
  printf("\nCOTIZACIONES POR CABALLO :\n");
  for(i = 0; i < caballos; i++) {
    printf(" - Caballo %d : %.2lf\n", i+1, info->cotizacion[i]);
  }
  sleep(2);
  printf("\nComienza la carrera!\n\n");
  printf("\t\t");
  for(i = 0; i < caballos; i++) printf("C%d\t", i+1);
  printf("\n");
  /*La carrera es impresa hasta que el padre haga acabado = 1*/
  while(acabado != 1){
    /*Espera a que el padre le avise para imprimir*/
    if(Down_Semaforo(semid, 0, SEM_UNDO) == ERROR) return ERROR;
    printf("Tiradas   \t");
    for(i = 0; i < caballos; i++) printf("%d\t", info->tiradas[i]);
    printf("\nRecorrido\t");
    for(i = 0; i < caballos; i++) printf("%d\t", info->recorridos[i]);
    printf("\n");
    acabado = info->acabado;
    /*Up del semaforo del padre*/
    if(Up_Semaforo(semid, 1, SEM_UNDO) == ERROR) return ERROR;
  }
  printf("\nLa carrera ha finalizado! Los apostadores con más ganancias son :\n\n");
  /*Actualizamos el pago a 0 de los perdedores*/
  for(i = 0; i < info->cont; i++){
    if(info->recorridos[info->mensaje[i].caballo] != longitud) {
      info->mensaje[i].pago = 0;
    }
  }
  /*Reservamos memoria para las tablas de los diez mayores ganadores*/
  ganadores = (int*)malloc(sizeof(int)*info->cont);
  if(!ganadores) return ERROR;
  ganancias = (double*)malloc(sizeof(double)*info->cont);
  if(!ganancias){
    free(ganadores);
    return ERROR;
  }
  for(i = 0; i < info->cont; i++){
    ganadores[i] = i;
    ganancias[i] = info->mensaje[i].pago;
  }
  /*Ordenamos los ganadores e imprimimos*/
  if(BubbleSort(ganadores, ganancias, 0, info->cont-1) < 0) {
    free(ganadores);
    free(ganancias);
    return ERROR;
  }
  /*Si el numero de apostadores es menor a 10 solo imprimimos esos*/
  if(info->cont < 10) cont = info->cont;
  for(i = 0; i < cont; i++) {
    printf("\t%dº %s con %.2lf euros\n", i+1, info->mensaje[ganadores[i]].nombre, ganancias[i]);
  }
  free(ganadores);
  free(ganancias);
  printf("\nEn breves daremos los resultados...\n");

  /*Esperamos 15 segundos hasta realizar el report*/
  alarm(15);
  sigsuspend(&mask);
  printf("\t\t ______________________ \n");
  printf("\t\t|                      |\n");
  printf("\t\t| REPORT DE LA CARRERA |\n");
  printf("\t\t|______________________|\n\n");
  /*Imprimimos las apuestas realizadas*/
  printf("Apuestas realizadas : \n");
  for(i = 0; i < info->cont; i++) {
    printf(" - %s : %.2lf euros al ", info->mensaje[i].nombre, info->mensaje[i].apuesta);
    printf("Caballo %d (%.2lf) ", info->mensaje[i].caballo+1, info->mensaje[i].cotizacion);
    printf("-> Ventanilla %d\n", info->mensaje[i].ventanilla+1);
  }
  /*Imprimimos el resultado de los caballos*/
  printf("\n\tResultados de la carrera : \n");
  for(i = 0; i < caballos; i++) {
    printf("\t - Caballo %d : %d metros recorridos", i+1, info->recorridos[i]);
    if(info->recorridos[i] == longitud) printf(" 1ª Posición\n");
    else {printf("\n");}
  }
  /*Imprimimos el resultado de las apuestas*/
  printf("\n\tResultados de las apuestas : \n\n\t\tApuesta\t\tBeneficio\tDinero Restante\n");
  for(i = 0; i < info->cont; i++) {
    printf("%s\t%.2lf\t", info->mensaje[i].nombre, info->mensaje[i].apuesta);
    printf("\t%.2lf\t\t%.2lf\n", info->mensaje[i].pago, dinero - info->mensaje[i].apuesta);
  }
  return OK;
}

/**
*@brief Funcion encargada de crear apostadores y mandar mensajes a las ventanillas
*@param dinero : dinero maximo por apostador introducido por teclado
*@param caballos : numero de caballos
*@param apostadores : numero de apostadores introducido por teclado
*@param id_cola : id de la cola de mensajes
*@return OK o ERROR
*/
int apostador(int dinero, int caballos, int apostadores, int id_cola){
  int i;
  Msg_apuesta mensaje;

  /*Control de errores*/
  if(caballos < 0 || apostadores < 0) return ERROR;
  mensaje.mtype = 1;
  syslog(LOG_INFO, "Apostadores mandando mensajes a la cola con id %d", id_cola);
  /*Enviamos un mensaje por cada apostador*/
  for(i = 0; i < apostadores; i++){
    sprintf(mensaje.nombre, "Apostador-%d", i+1);
    mensaje.apuesta = aleat_num(100, dinero*100)/100.0;
    mensaje.caballo = aleat_num(0, caballos-1);
    syslog(LOG_INFO, "%s apuesta %.2lf al caballo %d", mensaje.nombre, mensaje.apuesta, mensaje.caballo);
    if(msgsnd(id_cola, (struct msgbuf *)&mensaje, sizeof(Msg_apuesta) -sizeof(long) -sizeof(int), 0) < 0)return ERROR;
    sleep(1);
  }
  return OK;
}

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
int gestor(sigset_t mask, int id_cola, int caballos, int apostadores, int ventanillas, int id){
  pthread_t hilo[ventanillas];
  Hilo_args arg[ventanillas];
  int i;
  /*Control de errores*/
  if(caballos < 0 || apostadores < 0 || ventanillas < 0) return ERROR;
  /*Cogemos la memoria compartida*/
  Carrera_info *info = shmat(id, (char*)0, 0);
  if(!info) return ERROR;
  syslog(LOG_INFO, "Gestor inicializa la informacion de la memoria compartida %d", id);
  /*Inicializamos las apuestas de los caballos, las cotizacion y el total*/
  info->cont = 0;
  info->total_apuestas = 0;
  for(i = 0; i < caballos; i++){
    info->total_caballo[i] = 1;
    info->total_apuestas += info->total_caballo[i];
    info->cotizacion[i] = info->total_apuestas/info->total_caballo[i];
  }
  /*Inicializamos las ventanillas de apuestas*/
  for(i = 0; i < ventanillas; i++) {
    arg[i].id_mem = id;
    arg[i].num_ventana = i;
    arg[i].id_cola = id_cola;
    pthread_create(&hilo[i], NULL , (void *(*)(void *))ventanilla, &arg[i]);
  }
  syslog(LOG_INFO, "Ventanillas creadas");
  /*Esperamos a que el padre nos avise para acabar*/
  sigsuspend(&mask);

  /*Mandamos una señal a los hilos para que acaben y los esperamos*/
  for(i = 0; i < ventanillas; i++) pthread_kill(hilo[i], SIGTERM);
  for(i = 0; i < ventanillas; i++) pthread_join(hilo[i], NULL);
  syslog(LOG_INFO, "Gestor finaliza");
  return OK;
}

/**
*@brief Funcion de los hilos encargada de recibir las apuestas y
* agregarlas a la memoria compartida
*@param arg : estructura que contiene el id de la cola y el numero de ventanilla
*@return pthread_exit
*/
void *ventanilla(Hilo_args *arg){
  int id_cola;
  Msg_apuesta mensaje;
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

  /*Control de errores*/
  if(!arg) pthread_exit(NULL);
  /*Cogemos la memoria compartida*/
  Carrera_info *info = shmat(arg->id_mem, (char*)0, 0);
  if(!info) return NULL;
  /*Cogemos el id de la cola de mensajes*/
  id_cola = arg->id_cola;
  /*Recibimos un mensaje*/
  while(msgrcv(id_cola, (struct msgbuf *)&mensaje, sizeof(Msg_apuesta) - sizeof(long) - sizeof(int), 1, MSG_NOERROR) > 0){
    /*Hacemos down del semaforo de hilos*/
    pthread_mutex_lock(&mutex);

    /*Guardamos la cotizacion del caballo antes de apostar y el numero de ventana*/
    mensaje.cotizacion = info->cotizacion[mensaje.caballo];
    mensaje.pago = mensaje.apuesta*info->cotizacion[mensaje.caballo];
    mensaje.ventanilla = arg->num_ventana;
    info->mensaje[info->cont] = mensaje;

    /*Recalculamos el total de apuestas y las cotizaciones*/
    info->total_apuestas += mensaje.apuesta;
    info->total_caballo[mensaje.caballo] += mensaje.apuesta;
    info->cotizacion[mensaje.caballo] = info->total_apuestas/info->total_caballo[mensaje.caballo];
    info->cont++;
    syslog(LOG_INFO, "Ventanilla %d : %s %.2lf euros al caballo %d", mensaje.ventanilla+1, mensaje.nombre, mensaje.apuesta, mensaje.caballo+1);
    /*Hacemos down del semaforo de hilos*/
    pthread_mutex_unlock(&mutex);
  }
  shmdt((char*)info);
  pthread_exit(NULL);
}

/**
*@brief Funcion capturadora de las señales
*@param : SIGUSR1 para avisar al padre de que comienza la carrera
*         SIGUSR2 para avisar al gestor y al apostador de que acaben
*         SIGINT para acabar el programa con CTRL+C y borrar memoria
*/
void captura(int sennal){
  char key[50];
  int k;
  if(sennal == SIGUSR2){
    exit(EXIT_SUCCESS);
  }
  else if(sennal == SIGTERM){
    pthread_exit(NULL);
  }
  /*En caso de cancelar el programa eliminamos la zona de memoria compartida y los semaforos*/
  else if(sennal == SIGINT){
    k = ftok(FILEKEY, KEY);
    printf("Cancelando carrera %d\n", getpid());
    sprintf(key, "ipcrm -M %d", k);
    system(key);
    sprintf(key, "ipcrm -S %d", k);
    system(key);
    sprintf(key, "ipcrm -Q %d", k);
    system(key);
    /*Matamos al proceso*/
    kill(getpid(), SIGKILL);
    return;
  }
  return;
}

/**
*@brief Devuelve un numero aleatorio entre inf y sup
* En caso de pasar un numero negativo se cambia de signo
* En caso de que sup sea menor inf se permutan
*@param inf minimo numero aleatorio que puede salir
*@param sup maximo numero aleatorio que puede salir
*@return el numero aleatorio
*/
int aleat_num(int inf, int sup){
  int aux;
  /*En caso de que alguno sea negativo los cambia a positivo*/
  if(sup < 0) sup = -sup;
  if(inf < 0) inf = -inf;

  /*En caso de que sup sea menor que inf los permuta*/
  if( sup < inf ){
    aux = sup;
    sup = inf;
    inf = aux;
  }
  return inf+(rand()%(sup-inf+1));
}

/**
* Se utiliza para los 10 apostadores con mas ganancias
*@brief Funcion que ordena la tabla ganadores en funcion de la tabla pago
*@param ganadores : tabla de los apostadores en orden de llegada
*@param pago : tabla del dinero de los apostadores en orden de llegada
*@param ip : primera iteracion
*@param iu : ultima iteracion
*@return numero de iteraciones y ERROR en caso de error
*/
int BubbleSort(int* ganadores, double *pago, int ip, int iu){
	if(!ganadores || !pago || ip < 0 || iu < 0) return ERROR;
	int cont, i, j, aux;
	cont = 0;

	for(i = iu; i >= ip+1; i--){
		for(j = ip; j <= i-1; j++){
			cont++;
			if(pago[j] < pago[j+1]){
				aux = pago[j];
				pago[j] = pago[j+1];
	 			pago[j+1] = aux;
        aux = ganadores[j];
				ganadores[j] = ganadores[j+1];
	 			ganadores[j+1] = aux;
			}
		}
	}
	return cont;
}
