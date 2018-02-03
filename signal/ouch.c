#include <signal.h>
#include "tlpi_hdr.h"

static void sigHandler(int sig)
{
		printf("Ouch\n");
}

int main(int argc, char *argv[])
{
		int j;
		/*Establish handler for SIGINT*/
		if(signal(SIGINT, sigHandler) == SIG_ERR)
				errExit("signal");
		/*Loop continously waiting for signals to be delivered*/
		for(j=0;;j++)
		{
				printf("%d\n", j);
				sleep(3);  /*loop slowly*/
		}
}
