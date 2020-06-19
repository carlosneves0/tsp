//gera matriz
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char const *argv[])
{
	FILE *p;
	int tam;
	int j, i;
	scanf("%d", &tam);
	
	p=fopen("entrada1.txt","w");
	fprintf(p,"%d\n",tam);
	for (i = 0; i < tam; i++){
		for (j = 0; j < tam; j++){
			if(i==j)
				fprintf(p,"0 ");
			else
				fprintf(p,"%d ",rand()%10);
		}
		fprintf(p,"\n");
	}

	p=fopen("entrada2.txt","w");
	fprintf(p,"%d\n",tam*5);
	for (i = 0; i < tam*5; i++){
		for (j = 0; j < tam*5; j++){
			if(i==j)
				fprintf(p,"0 ");
			else
				fprintf(p,"%d ",rand()%10);
		}
		fprintf(p,"\n");
	}
	p=fopen("entrada3.txt","w");
	fprintf(p,"%d\n",tam*10);
	for (i = 0; i < tam*10; i++){
		for (j = 0; j < tam*10; j++){
			if(i==j)
				fprintf(p,"0 ");
			else
				fprintf(p,"%d ",rand()%10);
		}
		fprintf(p,"\n");
	}
	p=fopen("entrada4.txt","w");
	fprintf(p,"%d\n",tam*100);
	for (i = 0; i < tam*100; i++){
		for (j = 0; j < tam*100; j++){
			if(i==j)
				fprintf(p,"0 ");
			else
				fprintf(p,"%d ",rand()%10);
		}
		fprintf(p,"\n");
	}
	fclose(p);
	return 0;
}


