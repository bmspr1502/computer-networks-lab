#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{

    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[1024];
    if (argc < 3)
    {
        fprintf(stderr, "Usage of %s hostname port \n", argv[0]);
        exit(0);
    }

    //getting the port number
    portno = atoi(argv[2]);

    //socket file descriptor
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
        error("ERROR WHILE OPENING SOCKET\n");

    server = gethostbyname(argv[1]);
    if (server == NULL)
    {
        fprintf(stderr, "NO SUCH HOST, ERROR.\n");
        exit(0);
    }

    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);

    serv_addr.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR CONNECTING.\n");

    printf("Client: ");
    while (1)
    {
        bzero(buffer, 256);
        fgets(buffer, 255, stdin);

        n = write(sockfd, buffer, strlen(buffer));

        if (n < 0)
            error("ERROR WHILE WRITING IN SOCKET.");

        bzero(buffer, 256);

        n = read(sockfd, buffer, 255);

        if (n < 0)
            error("ERROR WHILE READING FROM SOCKET.");

        printf("Server: %s\n", buffer);

        int i = strncmp("Bye", buffer, 3);

        if (i == 0)
            break;
    }

    close(sockfd);
    return 0;
}