#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <string.h>
#include <errno.h>
#include <sys/shm.h>

#define FILEKEY "/bin/cat"
#define KEY 1300

struct info{
 char nombre[80];
 int id;
}

int main(void){
    info *i;
    int id_zone, key, n_hijos, i;
    pid_t pid;

    key = ftok(FILEKEY, KEY);
    if(key == -1){
        perror("Error de clave");
        exit(EXIT_FAILURE);
    }

    id = shmget(key, sizeof(int), IPC_CREAT | IPC_EXCL | SHM_R | SHM_W);
    if(id == -1){
        perror("Error en shmget");
        exit(EXIT_FAILURE);
    }

    for(i = 0; i < n_hijos; i++){
        pid = fork();

        /*En caso de error*/
        if(pid < 0){
            perror("Error en el fork");
            exit(EXIT_FAILURE);
        }

        else if(pid > 0){
            pause();
        }
    }

}
