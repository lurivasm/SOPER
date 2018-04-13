
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <time.h>

#define INF 100
#define MID 1000
#define SUP 2000

int aleat_num(int inf, int sup);
void *hilo1(void *arg);
void *hilo2(void *arg);

int main(void){
  char archivo[15] = "ejercicio4.txt";
  pthread_t h1, h2;

  pthread_create(&h1, NULL, hilo1, (void*)archivo);
  pthread_join(h1, NULL);

  pthread_create(&h2, NULL, hilo2, (void*)archivo);
  pthread_join(h2, NULL);

  exit(EXIT_SUCCESS);
}

void *hilo1(void *arg){
  int len, num, i;
  FILE *fich;

  if(!arg){
    perror("Error en el argumento");
    pthread_exit(NULL);
  }
  /*Abrimos el fichero*/
  fich = fopen((char*)arg, "w");
  if(!fich){
    perror("Error en fopen");
    pthread_exit(NULL);
  }

  /*Establecemos la longitud y escribimos en el fichero*/
  len = aleat_num(MID, SUP);
  for(i = 0; i < len-1; i++){
    num = aleat_num(INF, MID);
    fprintf(fich, "%d,", num);
  }
  num = aleat_num(INF, MID);
  fprintf(fich, "%d", num);
  /*Cerramos el fichero y salimos*/
  fclose(fich);
  pthread_exit(NULL);
}

void *hilo2(void *arg){
  int fich, size, i, res;
  char *buffer;
  struct stat stat_buff;

  if(!arg){
    perror("Error en el argumento");
    pthread_exit(NULL);
  }
  /*Abrimos el fichero para leer y escribir*/
  fich = open((char*)arg, O_RDWR);
  if(fich < 0){
    perror("Error en fopen");
    pthread_exit(NULL);
  }
  /*Vemos el tamaño*/
  if(fstat(fich, &stat_buff) < 0){
    perror("Error en fstat");
    pthread_exit(NULL);
  }
  size = stat_buff.st_size;

  /*Mapeamos*/
  buffer = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fich, 0);
  if(buffer == MAP_FAILED){
    perror("Error en mmap");
    pthread_exit(NULL);
  }
  printf("Hilo 2 imprime:\n");
  for(i = 0; i < size; i++){
    if(44 == buffer[i]) buffer[i] = 32;
    printf("%c", buffer[i]);
  }

  res = munmap(0, size);
  /*Liberamos y cerramos*/
  if(res == -1){
    perror("Error munmap");
  }
  close(fich);
  pthread_exit(NULL);
}


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
