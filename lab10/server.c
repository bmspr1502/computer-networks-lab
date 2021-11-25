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
    int socketfd = 0, clientfd = 0;
    struct sockaddr_in host_addr, client_addr;
    socklen_t length = sizeof(struct sockaddr_in);

    char buffer[1024];
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd < 0)
    {
        fprintf(stderr, "Error while Socket creation \n");
        return -1;
    }

    host_addr.sin_family = AF_INET;
    host_addr.sin_port = ntohs(PORT);
    inet_pton(AF_INET, "127.0.0.1", &host_addr.sin_addr);

    if (bind(socketfd, (struct sockaddr *)&host_addr, length) < 0)
    {
        fprintf(stderr, "Error while bind creation\n");
        return -1;
    }

    if (listen(socketfd, 5) < 0)
    {
        fprintf(stderr, "Error in listen port\n");
        return -1;
    }

    printf("<<Listening on %s : %d>>\n", inet_ntoa(host_addr.sin_addr), ntohs(host_addr.sin_port));

    while (1)
    {
        printf("==================================\n\n");

        clientfd = accept(socketfd, (struct sockaddr *)&host_addr, &length);

        if (clientfd < 0)
        {
            fprintf(stderr, "Error in accepting connection.\n");
            continue;
        }

        printf("<< Accepted connection. >>\n");

        int w, f;

        recv(clientfd, &w, sizeof(int), 0);
        printf(">> The given window size : %d\n", w);

        recv(clientfd, &f, sizeof(int), 0);
        printf(">> The given number of frames to transmit : %d\n", f);

        int i = 0, frames;
    S:
        recv(clientfd, &frames, sizeof(int), 0);

        if (frames == -9999)
        {
            printf("\n<< Sending acknowledgement of received frames \n");
            printf("==================================\n\n");
            close(clientfd);
            break;
        }
        i++;

        printf(" %d\t", frames);
        if (i == w)
        {
            i = 0;
            printf("\n<< Sending acknowledgement of above frames\n");
        }
        goto S;
    }
    close(socketfd);
    return 0;
}