#include<stdio.h>
#include<stdlib.h> 

//P=50,	L=20,	D=-10,	C=30,	S=40 

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
int main(void)
{
	int line = 0;
	char input[26];
	char output[26];
	char x;
	
	FILE *pToFileForm = fopen("input.txt", "r");
	FILE *f = fopen("mapper_output.txt", "w");

	printf("mappper_output.txt: \n");
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

	printf("\nover\n");
	system("pause"); 
	return 0;
}

