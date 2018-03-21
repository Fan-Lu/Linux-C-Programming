#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<math.h>
#include<string.h> 
#include<fcntl.h>

int main(int argc, char **argv)
{
	int length, fd1, fd2, rc;
	char *nodename = "/dev/tuxdrv";
	char message[] = "TESTING CHAR/DRIVER\n";
	fd1 = open(nodename, O_RDWR);
	printf(" opened file descriptor first time  = %d\n", fd1);
	close(fd1);
	exit(0);
}

