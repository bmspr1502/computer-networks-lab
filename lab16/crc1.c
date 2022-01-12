#include <stdio.h>
#include <string.h>
#define SIZE 128

int main(){
	char da[SIZE], di[SIZE], te[SIZE], tem[SIZE];
	
	//strcpy(da, msg->data);
	//strcpy(di, msg->divisor);
	
	printf("Enter Data: ");
	scanf("%s", da);
	printf("Enter divisor: ");
	scanf("%s", di);

	
	int i,j,l,m,n,data,div,t,k,e;
	l=div;
	t=0;
	k=0;
	
		
	data = strlen(da);
	div = strlen(di);
	for(i=0;i<data;i++)
	{
	  e=0;t=0;
	  for(j=1;j<div;j++)
		 {
		   if(((da[j]=='1')&&(di[j]=='1'))||((da[j]=='0')&&(di[j]=='0')))
		      {
		        tem[j-1]='0';
		        if(e!=1)
		           {
		             k=k+1;
		             t=t+1;
		             i=i+1;
		           }
		      }
		   else
		      {
		        tem[j-1]='1';
		        e=1;
		      }
		 }
	  j=0;
	  for(e=t;e<div-1;e++)
		 {
		   da[j]=tem[e];
		   j++;
		 }
	  for(j=j;j<div;j++)
		 {
		   if(l>=data+1)
		      {
		        da[j]=0;
		      }
		   else
		      {
		        da[j]=te[l];
		        l=l+1;
		      }
		 }
	}
	printf("\n The CRC BITS are ");
	for(i=0;i<div-1;i++)
	{
	  printf(" %c",tem[i]);
	  da[data+i] = tem[i];
	}
	da[data+div]='\0';

	return 0;
}
