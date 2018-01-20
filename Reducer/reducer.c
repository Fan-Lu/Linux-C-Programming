#include<unist.h>
#include<stdio.h>
#include<stdlib.h> 
#include<math.h>

//P=50,	L=20,	D=-10,	C=30,	S=40 

int main(void)
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
	printf("\nover\n");
	system("pause"); 
	return 0;	
}

