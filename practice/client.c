#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define PORT 35550
#define SIZE 1024

int main(){
    int serverfd;
    struct sockaddr_in server;
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

    if(connect(serverfd, (struct sockaddr *) &server, sizeof(server)) < 0){
        fprintf(stderr, "CONNECTION ERROR\n");
        exit(0);
    }else{
        printf("<< Socket has been connected to server successfully. >>\n");
	while(1){
		bzero(msg, SIZE);
		printf(">> Send Message: ");
		fgets(msg, SIZE, stdin);
		if(strncmp(msg, "exit", 4)==0){
                        close(serverfd);
                	break;
                }	
		send(serverfd, msg, SIZE, 0);

		bzero(msg, SIZE);
		recv(serverfd, msg, SIZE, 0);
		printf("<<Server: %s", msg);
	}
    }
}
