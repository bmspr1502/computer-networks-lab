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
    int serverfd, n;
    socklen_t s;
    char msg[size];
    struct sockaddr_in server, client;

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(PORT);
    // inet_pton(AF_INET,"127.0.0.1",&server.sin_addr);

    if ((serverfd = socket(AF_INET, SOCK_DGRAM, 0)) > 0)
        printf("\n-> Socket created\n");
    else
    {
        printf("\n-> Error in socket creation\n");
        exit(EXIT_FAILURE);
    }
    if (bind(serverfd, (struct sockaddr *)&server, sizeof(server)) == 0)
        printf("\n-> Bind successful\n");
    else
    {
        printf("\n-> Error in binding socket to the port\n");
        exit(EXIT_FAILURE);
    }
    // if(listen(serverfd,5) == 0)
    //     printf("\n-> Listening on %s : %d\n",inet_ntoa(server.sin_addr),PORT);
    // else{
    //     printf("\n-> Error in listening to the port\n");
    //     exit(EXIT_FAILURE);
    // }

    s = sizeof(struct sockaddr_in);
    n = recvfrom(serverfd, msg, size, 0, (struct sockaddr *)&client, &s);
    if (n < 0)
        fprintf(stderr, "Error in reading from socket\n");
    printf("Message from client :%s\n", msg);
    bzero(msg, size);

    strcpy(msg, "Ack from server\n");
    n = sendto(serverfd, msg, strlen(msg) + 1, 0, (struct sockadrr *)&client, s);
    if (n < 0)
        fprintf(stderr, "Error in writing to the socket\n");
    bzero(msg, size);

    close(serverfd);
    return 0;
}