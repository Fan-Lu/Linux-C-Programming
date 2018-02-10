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
#include <string.h>
#include <math.h>

static pthread_cond_t full;
static pthread_cond_t empty;
static pthread_mutex_t mtx;
int num_buffer; //number of buffers == number of reducers/users == number of comsumer threads
int num_slots; //number of slots in each buffer
char **buffer; //used to store the data from input
int tuple_size = 30; 
int whole_size; //whole size of each buffer
int *pos;  //current position
int *tup;  //num of tuples insert
int length = 26;  //tuple length
int num_insert = 0;

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
		char input[length];
		char output[length];
		char x;
		char user_id[num_buffer];  //use to identify which buffer to insert items
		char ID; //Current ID

		int j;
		for(j=0; j<num_buffer; j++)
		{
				user_id[j] = j + '0';
		}

		FILE *pToFileForm = fopen("input.txt", "r");
		FILE *f = fopen("mapper_output.txt", "w");

		printf("mappper_output.txt: \n");

		pthread_mutex_lock(&mtx);
		while(fgets(input,length,pToFileForm))
		{
			int i=length;
			for(i=length;i--;i>0)
			{
				output[i] = input[i];
			}
			char *add = &input[6];
			x = *add;
			ID = input[4];
			adjust(x, output);
		
			for(i=0; i<num_buffer; i++)
			{
					if(ID == user_id[i])
					{
							for(j=pos[i]; j<(length+pos[i]); j++)
							{
									if(output[j-pos[i]] == '\n')   //detect the end of output tuple
									{
											buffer[i][j] = '\n';
											pos[i] = j+1;
											break;
									}
									else
									{
											buffer[i][j] = output[j-pos[i]];
									}
							}
							tup[i]++;  //insert one tuple, will use it to compare to the num_slots
							num_insert++;			//number of tuples inserted
							
					}
			}
			printf("%d\n", num_insert);
			char *text = output;
			printf("%s", text);
			fprintf(f, output, text);
		}	
		pthread_mutex_unlock(&mtx);	
		fclose(f);
		fclose(pToFileForm);
		
		pthread_exit(NULL);

}

void *reducer(void *buf) //Comsumer
{
		char save[50][50];
		long buf_index;
		buf_index = (long)buf;

		int t=0;
		int line;
		for(; buffer[buf_index][t] != '\n'; t++)
		{
				save[line][t] = buffer[buf_index][t];
				t++;
				save[line][t] = '0';
				line++;
		}

		pthread_exit(NULL);
}


int main(int argc, char *argv[]) 
{
		int k,j;

		if(argc == 1)
		{
				printf("Please Enter Input\n");
		}
		else
		{
				num_slots = atoi(argv[1]); //buffer size
				num_buffer = atoi(argv[2]); //num use
		}

		int position[num_buffer], tuple_counter[num_buffer];

		whole_size = 400;
		/*create buffer for the communication between producer and consumer*/
		buffer = (char**)malloc(sizeof(char*)*num_buffer); //assign number of bufferi
		for(k=0; k<num_buffer; k++)
		{
				buffer[k] = (char*)malloc(sizeof(char)*whole_size); //assign the size of  space for each buffer
		}
		for(k=0; k<num_buffer; k++)  //initialize all the buffer
		{
				for(j=0; j<whole_size; j++)
						buffer[k][j] = 'o';
		}

		/*Assign storage and initialize values*/
		for(k=0; k<num_buffer; k++)
		{
				position[k] = 0;
				tuple_counter[k] = 0;
		}
		pos = position;
		tup = tuple_counter;

		void *status;
		pthread_attr_t attr;
		pthread_mutex_init(&mtx, NULL);
		pthread_t producer, consumer[num_buffer];

		/*Create threads*/
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
		/*Create producer thread;*/
		pthread_create(&producer, &attr, mapper, NULL);
		/*create consumer threads*/
		long i;
		for(i=0; i<num_buffer; i++)
		{
			pthread_create(&consumer[i], &attr, reducer, (void *)i); //pass ith buffer to be handle by ith thread
		}

		/*wait on other threads*/
		pthread_join(producer, &status);
		for(i=0; i<num_buffer; i++)
		{
				pthread_join(consumer[i], &status);
				//printf("main joined thread %ld\n", i);
		}

		pthread_mutex_destroy(&mtx);
		pthread_attr_destroy(&attr);
		pthread_exit(NULL);
}






