#include <conio.h>
#include <mman.h>
#include <stdlib.h>
#include <string.h>

/* shell */
int main(int argc, char *argv[])
{
	printf("Welcome to shell. my pid %d\n", getpid());
	printf("/ $> \n");
	//while(1);
	int pid = fork();
	if (pid > 0) {
		printf("I am father. my pid %d, my child %d\n", getpid(), pid);	
		
		int status;
		int pid;
		pid = _wait(&status);
		printf("my child pid %d status %d\n", pid, status);
		while(1);
	} else if (pid == 0) {
		printf("I am child. my pid %d\n", getpid());
		
		//while(1);
		
		const char *arg[4];
		
		arg[0] = "hello";
		arg[1] = "world";
		arg[2] = "xbook!";
		arg[3] = 0;
		
		execv("test", arg);	
	}
	return 0;
}
