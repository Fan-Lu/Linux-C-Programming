#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<math.h>
#include<string.h> 

void adjust(char A, char In[]);
void mapper();
void reducer();

int main()
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

/*
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
	*q = '\0';
	
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
	*n = '\0';
	//printf("%c\n",In);
} 

void mapper(void)
{
	int line = 0;
	char input[26];
	char output[26];
	char x;
	
	FILE *pToFileForm = fopen("input.txt", "r");
	FILE *f = fopen("mapper_output.txt", "w");

	printf("input.txt: \n");
	while (fgets(input,26,pToFileForm))
	{
		line++;
		int i=26;
		for(i;i--;i>0)
		{
			output[i]=input[i];
		}
		
		char *add = &input[6];
		x=*add;
		adjust(x,output);
		printf(output);
		char *text = output;
		fprintf(f, output, text);
	}	
	
	fclose(f);
	fclose(pToFileForm);
}

void reducer(void)
{
	int line = 0;
	char input[20000];
	char save[50][50];
	char compare[7];
	char standard[7];
	char output1[50][30];
	char output2[50];
	char x;
	
	
	FILE *pToFileForm = fopen("mapper_output.txt", "r");
	
	printf("input.txt: \n");
	while (fgets(input,20000,pToFileForm))//read information from the file and save the data into the struct
	{
		int i=0;
		while(1)
		{		
			int g=0;
			for(;input[i]!=')';i++,g++)
			{
				save[line][g]=input[i];	
			}
			save[line][g]=')';
			i++;
			g++;
			save[line][g]='\0';
			if(line==49)
			{
				break;
			}
			//printf(save[line]);
			line++;
		}
	}	
	fclose(pToFileForm);
	
	int j=0,k=0,l=0,t=0,y=0,z=0;
	for(j=0;j<50;j++)
	{ 	
	 	if(save[j][0]=='X')
	 		j++;
		for(l=0;l<7;l++)
		{
			standard[l]=save[j][1+l];
		}
		for(t=j+1;t<50;t++)
		{		
			for(z=0;z<7;z++)
			{
				compare[z]=save[t][1+z];
			}
			int h=1,o=0;
			if((h=strcmp(compare,standard))==0)
			{
				int num1,num2;
				if(save[j][21]=='-')
				{
					num1 = -1;
					num1 = num1*((save[j][22]-'0')*10+save[j][23]-'0');
				}
				else
				{
					num1 = 1;
					num1 = num1*((save[j][21]-'0')*10+save[j][22]-'0');
				}
				if(save[t][21]=='-')
				{
					num2 = -1;
					num2 = num2*((save[t][22]-'0')*10+save[t][23]-'0');
				}
				else
				{
					num2 = 1;
					num2 = num2*((save[t][21]-'0')*10+save[t][22]-'0');
				}
				num1 = num1 + num2;
				//printf("%d\n",num1);
				if(num1<0)
				{
					save[j][22]='-';
					num1=num1*(-1);
					save[j][22]=num1/10+'0';
					save[j][23]=num1%10+'0';
					save[j][24]=')';
					save[j][25]='\0';
				}
				else
				{
					save[j][21]=num1/10+'0';
					save[j][22]=num1%10+'0';
					save[j][23]=')';
					save[j][24]='\0';
				}
				save[t][0]='X'; 		
			}
		}
	}	
	int r=0;
	FILE *fp = fopen("reduce_output.txt", "w");
	while(r!=50)
	{
		if(save[r][0]!='X')
		{
			int d=0,f=0,g=0;
			char mid[50];
			mid[0]='(';
			printf("%c",mid[0]);
			for(d=1;save[r][d-1]!=')';d++)
			{
				mid[d]=save[r][d];
				printf("%c",mid[d]);
			}
			mid[d]='\0';
			char *text = mid;
			fprintf(fp,mid,text);
		}
		r++;			
	}
	fclose(fp);	
}
*/
