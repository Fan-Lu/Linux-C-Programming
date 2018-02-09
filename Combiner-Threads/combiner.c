/******************************************************
 * FILE: combiner.c
 * DESCRIPTION:
 *
 * AUTHOR: FAN LU
 * LAST REVISED: 02/06/2018
 * ***************************************************/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

pthread_cond_t full;
pthread_cond_t empty;
pthread_mutex_t mtx;
int N; //buffer size
int M; //Num of item to insert, i.e. tuple input
int num_user; //equals number of buffers
char buffer[50][26];

struct All_Pass{
		char (*buff)[30];  //buffer pointer
		int buff_size;
};

void adjust(char A, char In[])
{
	char *P = ",50)";
	char *L = ",20)";
	char *D = ",-10)";
	char *C = ",30)";
	char *S = ",40)";
	char *p,*q,*m,*n;
	int i = 0;
	
	switch(A)
	{
		case 'P': m = P;break;
		case 'L': m = L;break;
		case 'D': m = D;break;
		case 'C': m = C;break;
		case 'S': m = S;break;
		default: break;
	}
	
	for(p=In,q=In;*p!='\0';p++)
	{
		i++;
		*q=*p;
		if(i!=7 && i!=8)
  		{
			q++;
		}		
	}
	*q = '\n';
	q++;
	q = '\0';
	
	for(p=In,n=In;*(p+1)!=')';p++)
	{
		*n=*p;
		n++;
	}
	
	for(m;*m!='\0';m++)
	{
		*n=*m;
		n++;
	}
	*n = '\n';
	n++;
	*n = '\0';
}


void *mapper(void *param)  //Producer
{
		int line = 0;
		char input[26];
		char output[26];
		char x;
	
		FILE *pToFileForm = fopen("input.txt", "r");
		FILE *f = fopen("mapper_output.txt", "w");

		//printf("mappper_output.txt: \n");
		while (fgets(input,26,pToFileForm))
		{
			line++;
			int i=26;
			for(i;i--;i>0)
			{
				output[i]=input[i];
			}
		
			char *add = &input[6];
			x = *add;
			adjust(x, output);
			char *text = output;
			//printf("%s", text);
			//buffer[line] = output[0];
			fprintf(f, output, text);
		}	
	
		fclose(f);
		fclose(pToFileForm);
		//printf("\nover\n");
		
		pthread_exit(NULL);

}

void *reducer(void *buf) //Comsumer
{
		//long tid;
		//tid = (long)param;
		//printf("Create reducer thread %ld\n", tid);

		struct All_Pass *my_buf;
		my_buf = (struct All_Pass *)buf;
		//printf("%d\n",my_buf->buff_size);

		int i;
		for(i=0; i<M; i++)  //M is the number of item to be inserted
		{
				pthread_mutex_lock(&mtx);
				//printf("Buff Contex: %s\n", (*mybuf.buff)[0][0]);
				//critical region
				pthread_mutex_unlock(&mtx);
		}
		pthread_exit(NULL);
}


int main(int argc, char *argv[]) 
{
		if(argc == 1)
		{
				printf("Please Enter Input\n");
		}
		else
		{
				N = atoi(argv[1]); //buffer size
				num_user = atoi(argv[2]); //num user
		}

		char buffer[N][30];
		struct All_Pass all_pass;
		all_pass.buff = buffer;
		//printf("%c\n", (all_pass.buff)[0][0]);
		all_pass.buff_size = N;

		void *status;
		pthread_attr_t attr;
		pthread_mutex_init(&mtx, NULL);
		pthread_t mapper_thread, reducer_thread[num_user];

		/*Create threads*/
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
		pthread_create(&mapper_thread, &attr, mapper, (void *)&all_pass);
		long i;
		for(i=0; i<num_user; i++)
		{
			pthread_create(&reducer_thread[i], &attr, reducer, (void *)&all_pass);
		}

		/*wait on other threads*/
		for(i=0; i<num_user; i++)
		{
				pthread_join(reducer_thread[i], &status);
				printf("main joined thread %ld\n", i);
		}

		pthread_mutex_destroy(&mtx);
		pthread_attr_destroy(&attr);
		pthread_exit(NULL);
}






