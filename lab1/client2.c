#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#define MAX 80
#define SA struct sockaddr

void func(int sockfd)
{
    char buff[MAX];
    int n;
    while (1)
    {
        //first the client should send the message
        bzero(buff, sizeof(buff));

        printf("Enter the string : ");
        n = 0;
        while ((buff[n++] = getchar()) != '\n')
            ;

        // sending the written message through the socket
        write(sockfd, buff, sizeof(buff));

        // clearing the buffer
        bzero(buff, sizeof(buff));

        // getting the string from the server and printing
        read(sockfd, buff, sizeof(buff));
        printf("From Server : %s", buff);

        if ((strncmp(buff, "exit", 4)) == 0)
        {
            printf("Client Exit...\n");
            break;
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
        return 1;
    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;

    // socket create and varification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    int PORT = atoi(argv[1]);
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    // connect the client socket to server socket
    if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0)
    {
        printf("connection with the server failed...\n");
        exit(0);
    }
    else
        printf("connected to the server..\n");

    // function for chat
    func(sockfd);

    // close the socket
    close(sockfd);
}
