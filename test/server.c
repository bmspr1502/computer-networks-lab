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

    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("<<NOT ABLE TO BIND TO PORTNO>>\n");
        exit(0);
    }

    listen(sockfd, 5);

    while (1)
    {
        int clientfd, id;
        clientfd = accept(sockfd, NULL, NULL);
        if ((id = fork()) == 0)
        {
            char msg[SIZE];
            if (clientfd > 0)
            {
                printf("Connection established.\n");

                recv(clientfd, msg, SIZE, 0);
                printf(">>Client: %s\n", msg);
                bzero(msg, SIZE);

                printf("Write message: ");
                fgets(msg, SIZE, stdin);
                send(clientfd, msg, strlen(msg) + 1, 0);
            }

            close(clientfd);
        }
    }
}