#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <math.h>
#include <iostream>

#define SIZE 128
#define PORT 1502

struct message{
	char data[SIZE];
	char divisor[SIZE];
	};
int checkCRC(struct message *msg);
	
int main(){
	int servfd, clientfd, stat;
	struct sockaddr_in host_addr, client_addr;
    servfd = socket(AF_INET, SOCK_STREAM, 0);
    int length;
    if (servfd < 0)
    {
        fprintf(stderr, "Error in socket creation.\n");
        return -1;
    }

    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(PORT);
    host_addr.sin_addr.s_addr = INADDR_ANY;
    
    
    if (bind(servfd, (struct sockaddr *)&host_addr, sizeof(host_addr)) < 0)
    {
        fprintf(stderr, "Error in binding port.\n");
        return -1;
    }

    if (listen(servfd, 5) < 0)
    {
        fprintf(stderr, "Error in listening on port.\n");
        return -1;
    }
    fprintf(stdout, "SERVER STARTED\nListening on %s:%d.\n\n", inet_ntoa(host_addr.sin_addr), ntohs(host_addr.sin_port));
    
    while(1){
    	clientfd = accept(servfd, (struct sockaddr *)&client_addr, &length);
    	if (clientfd < 0)
        {
            fprintf(stderr, "Error in accepting connection.\n");
            return -1;
        }
        fprintf(stdout, "New Connection established.\n");
        
        struct message msg;
        stat = recv(clientfd, &msg, sizeof(msg), 0);
        if(checkCRC(&msg)){
        	send(clientfd, "NO ERROR", 10, 0);
        }else{
        	send(clientfd, "ERROR CODE", 12, 0);
        }
        printf("<< Closing connection >>");
        close(clientfd);
    }
    
    close(servfd);
	return 0;
}


string toBin(long long int num){
	string bin = "";
	while (num){
		if (num & 1)
			bin = "1" + bin;
		else
			bin = "0" + bin;
		num = num>>1;
	}
	return bin;
}

long long int toDec(string bin){
	long long int num = 0;
	for (int i=0; i<bin.length(); i++){
		if (bin.at(i)=='1')
			num += 1 << (bin.length() - i - 1);
	}
	return num;
}

void CRC(string dataword, string generator){
	int l_gen = generator.length();
	long long int gen = toDec(generator);

	long long int dword = toDec(dataword);

	long long int dividend = dword << (l_gen-1);	

	int shft = (int) ceill(log2l(dividend+1)) - l_gen;
	long long int rem;

	while ((dividend >= gen) || (shft >= 0)){
		rem = (dividend >> shft) ^ gen;			
		dividend = (dividend & ((1 << shft) - 1)) | (rem << shft);
		shft = (int) ceill(log2l(dividend + 1)) - l_gen;
	}
	long long int codeword = (dword << (l_gen - 1)) | dividend;
	cout << "Remainder: " << toBin(dividend) << endl;
	cout << "Encoded data : " << toBin(codeword) << endl;
}


int checkCRC(struct message *msg){
	char da[SIZE], di[SIZE], te[SIZE], tem[SIZE];
	
	printf(">> Recieved data: %s\n", msg->data);
	printf(">> Recieved divisor: %s\n", msg->divisor);
	
	strcpy(da, msg->data);
	strcpy(di, msg->divisor);
	
	int i,j,l,m,n,data,div,t,k,e;
	data = strlen(da);
	div = strlen(di);
	l=div;
	t=0;
	k=0;

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
	printf("\n The CRC reminder BITS are ");
	int flag = 1;
	for(i=0;i<div-1;i++)
	{
	  printf(" %c",tem[i]);
	  if(tem[i]=='1'){
	  		flag = 0;
	  		}
	}
	printf("\n");
	if(flag){
		printf("\nThere was no error while transmitting. \n");
		}else{
		printf("\nThere was error while transmitting. \n");
		}

	return flag;
	}
