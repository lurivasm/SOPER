
#include "semaforos.h"


  union semun {
   int val;
   struct semid_ds *semstat;
   unsigned short *array;
   } arg;



/**
*@brief Inicializa los semaforos inficados
*@param semid : identificador del grupo de semaforos
*@param array : valores iniciales del semaforo
*@return OK / ERROR
*/
int Inicializar_Semaforo(int semid, unsigned short *array){
  if(semid < 0 || !array){
    return ERROR;
  }
  int i;
  for(i = 0; i < sizeof(array)/sizeof(unsigned short); i++){
    arg.array[i] = array[i];
  }
  if(semctl (semid, 0, SETALL, arg) < 0){
    return ERROR;
  }
  return OK;
}

/**
*@brief Elimina los semaforos indicados
*@param semid : identificador del grupo de semaforos
*@return OK / ERROR
*/
int Borrar_Semaforo(int semid){
  if(semid < 0){
    return ERROR;
  }
  if(semctl(semid, 0, IPC_RMID) < 0){
    return ERROR;
  }
  return OK;
}

/**
*@brief Crea los semaforos indicados
*@param key : clave precompartida del semaforo
*@param size : numero de semaforos a crear
*@param semid : identificador del grupo de semaforos
*@return OK / ERROR
*/
int Crear_Semaforo(key_t key, int size, int *semid){

  *semid = semget(key, size, IPC_CREAT | IPC_EXCL | SHM_R | SHM_W);
  if((*semid == -1) && (errno == EEXIST)){
    return 1;
  }
  if(*semid == -1){
    return ERROR;
  }
  return 0;
}

/**
*@brief Hace un down de un semaforo
*@param id : identificador del grupo de semaforos
*@param num_sem : numero del semaforo del cual hacemos el down
*@param undo : flags
*@return OK / ERROR
*/
int Down_Semaforo(int id, int num_sem, int undo){
  if(id < 0 || num_sem < 0 || undo < 0){
    return ERROR;
  }
  struct sembuf sem_oper;
  sem_oper.sem_num = num_sem;
  sem_oper.sem_op =-1;
  sem_oper.sem_flg = undo;
  if(semop (id, &sem_oper, 1) < 0){
    return ERROR;
  }
  return OK;
}

/**
*@brief Hace un down de un grupo de semaforos
*@param id : identificador del grupo de semaforos
*@param size : numero de semaforos para hacer un down
*@param undo : flags
*@param active : semaforos involucrados
*@return OK / ERROR
*/
int DownMultiple_Semaforo(int id,int size,int undo,int *active){
  if(id < 0 || size < 0 || undo < 0 || !active){
    return ERROR;
  }
  int i;
  struct sembuf sem_oper;
  sem_oper.sem_op =-1;
  sem_oper.sem_flg = undo;

  for(i = 0 ; i < size ; i++){
    sem_oper.sem_num = active[i];
    if(semop (id, &sem_oper, 1) < 0){
      return ERROR;
    }
  }
  return OK;
}

/**
*@brief Hace un up de un semaforo
*@param id : identificador del grupo de semaforos
*@param num_sem : numero del semaforo del cual hacemos el down
*@param undo : flags
*@return OK / ERROR
*/
int Up_Semaforo(int id, int num_sem, int undo){
  if(id < 0 || num_sem < 0 || undo < 0){
    return ERROR;
  }
  struct sembuf sem_oper;
  sem_oper.sem_num = num_sem;
  sem_oper.sem_op = 1;
  sem_oper.sem_flg = undo;
  if(semop (id, &sem_oper, 1) < 0){
    return ERROR;
  }
  return OK;
}

/**
*@brief Hace un up de un grupo de semaforos
*@param id : identificador del grupo de semaforos
*@param size : numero de semaforos para hacer un down
*@param undo : flags
*@param active : semaforos involucrados
*@return OK / ERROR
*/
int UpMultiple_Semaforo(int id,int size, int undo, int *active){
  if(id < 0 || size < 0 || undo < 0 || !active){
    return ERROR;
  }
  int i;
  struct sembuf sem_oper;
  sem_oper.sem_op = 1;
  sem_oper.sem_flg = undo;

  for(i = 0 ; i < size ; i++){
    sem_oper.sem_num = active[i];
    if(semop (id, &sem_oper, 1) < 0){
      return ERROR;
    }
  }
  return OK;
}
