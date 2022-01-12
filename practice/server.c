#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>


#define PORT 35550
#define SIZE 1024

int main(){
	int serverfd, clientfd;
     	struct sockaddr_in server, client;
	char msg[SIZE];

	serverfd = socket(AF_INET, SOCK_STREAM, 0);

    	if(serverfd<=0){
        	fprintf(stderr, "Socket creation error\n");
	        exit(0);
	}else{
        	printf("<< Socket created successflly.>>\n");
    	}	

	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);

	if(bind(serverfd, (struct sockaddr *) &server, sizeof(server)) < 0){
		fprintf(stderr, "BINDING ERROR. (PORT NOT OPEN) \n");
		exit(0);
	}else{
		printf("<< Binded, ready to accept connections. >>\n");
	}

	listen(serverfd, 5);
	
	socklen_t clilen = sizeof(client);
	
	while(1){
		
		clientfd = accept(serverfd, (struct sockaddr *) &client, &clilen);

		if(clientfd<0){
			fprintf(stderr, "ERROR WHILE ACCEPTING \n");
			exit(0);
		}else{
			if(fork()==0){
				close(serverfd);
			printf("<<CONNECTED WITH %d>>\n", ntohs(client.sin_port));
			while(1){
				bzero(msg, SIZE);
				recv(clientfd, msg, SIZE, 0);
				printf("<<Client(%d): %s", ntohs(client.sin_port), msg);
				if(strncmp(msg, "exit", 4)==0){
					close(clientfd);
					break;
				}

				printf(">>Send to Client%d: ", ntohs(client.sin_port));

				bzero(msg, SIZE);
				fgets(msg, SIZE, stdin);
				send(clientfd, msg, SIZE, 0);
			}
			printf("<<CONNECTION CLOSED WITH %d>>\n\n", ntohs(client.sin_port));
			break;
			}
		}
	}

	return 0;
}
