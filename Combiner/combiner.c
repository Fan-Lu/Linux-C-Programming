#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<math.h>
#include<string.h> 

int main(void)
{
	int n,fd[2];
	char buf[100];
	
	if(pipe(fd)==-1)
	{
		perror("pipe error");
	}

	pid_t pid_1 = fork();
	if(pid_1 < 0)
	{
		perror("fork 1 error");
	}
	else if(pid_1 == 0)//1st child process: write to the pipe
	{
		if(close(fd[0]) == -1)//close read end
			perror("close 1 error"); 
		
		if(fd[1] != STDOUT_FILENO)
		{
			if(dup2(fd[1],STDOUT_FILENO) == -1)
				perror("dup2 1 error");
			if(close(fd[1]) == -1)
				perror("close 2 error"); 
		}

		system("cc -o ./a.out ./mapper.c");
		execl("./a.out","a.out",NULL);
		perror("exec mapper");			
			
	}

	pid_t pid_2 = fork();
	if(pid_2 < 0)
	{
		perror("fork 2 error");
	}
	else if(pid_2 == 0)//2nd child process: read from the pipe
	{
		if(close(fd[1]) == -1)//close write end
			perror("close 3 error"); 

		if(fd[0] != STDIN_FILENO)
		{
			if(dup2(fd[0],STDOUT_FILENO) == -1)
				perror("dup2 1 error");
			if(close(fd[0]) == -1)
				perror("close 4 error"); 
		}

		system("cc -o ./a.out ./reducer.c");
		execl("./a.out","a.out",NULL);
	}
		
	if(close(fd[0]) == -1)
		perror("close 5 error");
	if(close(fd[1]) == -1)
		perror("close 6 error");

	if(wait(NULL) == -1)
		perror("wait 1 error");
	if(wait(NULL) == -1)
		perror("wait 2 error");

	exit(EXIT_SUCCESS);

	return 0;
}

