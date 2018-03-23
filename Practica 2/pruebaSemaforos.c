#include "semaforos.h"
#define SEMKEY 75798
#define NSEMAFOROS 2
int main ( )
{
 /*
 * Declaración de variables
 */
 int semid; /* ID de la lista de semáforos */
 unsigned short *array;
 union semun {
 int val;
 struct semid_ds *semstat;
 unsigned short *array;
 } arg;
 /*
 * Creamos una lista o conjunto con dos semáforos
 */
 int a;
 if((a = Crear_Semaforo(SEMKEY, NSEMAFOROS, &semid)) == ERROR){
  printf("ERROR 1\n");
  return -1;
 }
 printf("%d\n",a);
 /*
 * Inicializamos los semáforos
 */
 array = (unsigned short *)malloc(sizeof(short)*NSEMAFOROS);
 array[0] = 1;
 array[1] = 1;
 if(Inicializar_Semaforo( semid, array) == ERROR){
   Borrar_Semaforo(semid);
   printf("ERROR 2\n");
   return -1;
 }
 /*
 * Operamos sobre los semáforos
 */
if(Down_Semaforo(semid, 0, SEM_UNDO ) == ERROR){
   Borrar_Semaforo(semid);
  printf("ERROR 3\n");
  return -1;
}
if(Up_Semaforo(semid, 1, SEM_UNDO ) == ERROR){
  Borrar_Semaforo(semid);
  printf("ERROR 4\n");
  return -1;
}
 /*
 * Veamos los valores de los semáforos
 */
 semctl (semid, NSEMAFOROS, GETALL, arg);
 printf ("Los valores de los semáforos son %d y %d\n", arg.array [0], arg.array[1]);
 /* Eliminar la lista de semáforos */
 if(Borrar_Semaforo(semid) == ERROR){
   printf("ERROR 5\n");
   return -1;
 }
 free(array);
 return 0;
}/* fin de la función main */
