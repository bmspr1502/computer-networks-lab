#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>


#define SIZE 128
#define PORT 1502


struct message{
	char data[SIZE];
	char divisor[SIZE];
	};
	
void clientCRC(struct message *msg);
int main(){

	int sockfd = 0, stat;
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(sockfd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        printf("\nError while connecting, exiting...\n");
        return -1;
    }
        fprintf(stdout, "<< Connection established with %s:%d >>\n", inet_ntoa(server_address.sin_addr), ntohs(server_address.sin_port));
    
    struct message msg;
	printf("\n>> Enter the message (in binary): ");
	scanf("%s", msg.data);
	
	printf("\n>> Enter the divisor: ");
	scanf("%s", msg.divisor);
	
	clientCRC(&msg);
	
	stat = send(sockfd, &msg, sizeof(msg), 0);
    if (stat < 0)
    {
        printf("\nError while sending, exiting...\n");
        return -1;
    }
	
	
	char text[SIZE];
	
	stat = recv(sockfd, text, SIZE, 0);
	if (stat < 0)
    {
        printf("\nError while receiving data, exiting...\n");
        return -1;
    }
    
    printf("\n<< Received: %s\n", text);
	close(sockfd);
	return 0;
}


void clientCRC(struct message *msg){
	char da[SIZE], di[SIZE], te[SIZE], tem[SIZE];
	
	strcpy(da, msg->data);
	strcpy(di, msg->divisor);
	
	int i,j,l,m,n,data,div,t,k,e;
	data = strlen(da);
	div = strlen(di);
	l=div;
	t=0;
	k=0;
	for(i=0; i<div-1; i++){
		da[data+i]='0';
		}
	da[data+div-1]='\0';
	
	printf(">> Before division adding div-1 zeros, data = %s\n", da);
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
	  msg->data[data+i]=tem[i];
	}
	msg->data[data+div-1]='\0';
	printf("\n Modified DATA: %s\n", msg->data);
	printf("\n");

	}
