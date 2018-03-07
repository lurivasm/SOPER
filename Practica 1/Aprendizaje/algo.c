#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

void main(void){

	char	*prog[]	= {"ls", "-la",	NULL};
	execl("/bin/ls", "ls", "-la", NULL);
	perror("fallo en exec");
	exit(EXIT_FAILURE);
}
