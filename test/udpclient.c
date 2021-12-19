#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 7896
#define size 1000

int main()
{
    int clientfd, n;
    char msg[size];
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(PORT);

    if ((clientfd = socket(AF_INET, SOCK_DGRAM, 0)) > 0)
        printf("\n-> Socket created\n");
    else
        printf("\n-> Error in socket creation\n");

    socklen_t s = sizeof(struct sockaddr_in);
    n = sendto(clientfd, "Hello", strlen("Hello"), 0, (struct sockadrr *)&server, s);
    if (n < 0)
        fprintf(stderr, "Error in writing to the socket\n");

    n = recvfrom(clientfd, msg, size, 0, (struct sockaddr *)&server, &s);
    if (n < 0)
        fprintf(stderr, "Error in reading from socket\n");
    printf("Message from server :%s\n", msg);
    bzero(msg, size);

    close(clientfd);
    return 0;
}