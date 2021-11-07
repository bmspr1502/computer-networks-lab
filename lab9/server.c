#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define PORT 1502

int main()
{
    // OPEN PORT TO ACCEPT CONNECTIONS
    int socketfd = 0, clientfd = 0;
    struct sockaddr_in host_addr, client_addr;
    socklen_t length = sizeof(struct sockaddr_in);

    char buffer[128], acknowledgement[20];

    socketfd = socket(AF_INET, SOCK_STREAM, 0);

    if (socketfd < 0)
    {
        fprintf(stderr, "Error in creating socket.\n");
        return -1;
    }

    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &host_addr.sin_addr);

    if (bind(socketfd, (struct sockaddr *)&host_addr, length) < 0)
    {
        fprintf(stderr, "Error in binding socket to port.\n");
        return -1;
    }

    // LISTEN ON THE PORT
    if (listen(socketfd, 5) < 0)
    {
        fprintf(stderr, "Error in listening on %s:%d.\n", inet_ntoa(host_addr.sin_addr),
                ntohs(host_addr.sin_port));
        return -1;
    }

    printf(">> Listening on %s:%d.<<\n", inet_ntoa(host_addr.sin_addr), ntohs(host_addr.sin_port));

    while (1)
    {
        int clientfd = accept(socketfd, (struct sockaddr *)&host_addr, &length);
        if (clientfd < 0)
        {
            fprintf(stderr, "Error in accepting connection.\n");
            continue;
        }

        // ON ACCEPTING THE CONNECTION
        int limit;
        printf("<< Accepted connection.\n");
        recv(clientfd, &limit, sizeof(int), 0);
        printf("%d\n", limit);
        fflush(stdout);

        int packet = 1;
        char receivebuffer[2];
        while (1)
        {
            int j, temp;
            // RECEIVE THE PACKET
            printf("<< Receiving : ");
            for (j = 0; j < packet; j++)
            {
                recv(clientfd, receivebuffer, sizeof(receivebuffer), 0);
                if (strncmp(receivebuffer, "$", sizeof("$")) == 0)
                {
                    break;
                }
                printf("%s", receivebuffer);
            }

            printf("\n<< End of stream. %d packets.\n\n", j);

            char acknowledgement[10];

            // SEND ACKNOWLEDGEMENT TO THE CLIENT
            temp = sprintf(acknowledgement, "ACK %d", j);
            acknowledgement[temp] = '\0';

            printf(">> Sending acknowledgment for %d packets.\n", j);

            send(clientfd, acknowledgement, strlen(acknowledgement) + 1, 0);

            if (j < packet)
            {
                close(clientfd);
                break;
            }

            packet = limit;
        }
    }
    close(socketfd);
    return 0;
}