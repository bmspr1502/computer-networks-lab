#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <time.h>

#define SIZE 1024
#define PORTNO 1502

int main()
{
    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server;

    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORTNO);
    char msg[SIZE];

    if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("\n>>SOCKET CONNETION FAILED.<<\n");
        exit(0);
    }
    else
    {
        printf("Write a message: ");
        fgets(msg, SIZE, stdin);

        write(sockfd, msg, strlen(msg) + 1);
        bzero(msg, SIZE);
        read(sockfd, msg, SIZE);

        printf("From Server: %s\n", msg);
    }
}