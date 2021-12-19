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
    int socketfd = 0;
    struct sockaddr_in host_addr;
    socklen_t length = sizeof(struct sockaddr_in);

    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd < 0)
    {
        fprintf(stderr, "Error in creating socket.\n");
        return -1;
    }

    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &host_addr.sin_addr);

    if (connect(socketfd, (struct sockaddr *)&host_addr, length) < 0)
    {
        fprintf(stderr, "Error in connecting to server.\n");
        return -1;
    }

    printf("<< Connection has been made with the server. >>\n");

    int w, f;

    printf("\n>> Enter the window size : ");
    scanf("%d", &w);

    send(socketfd, &w, sizeof(int), 0);

    printf("\n>> Enter the number of frames to transmit: ");
    scanf("%d", &f);
    send(socketfd, &f, sizeof(int), 0);

    int frames[f], temp[w];
    printf("\nEnter %d frames : ", f);
    for (int i = 0; i < f; i++)
    {
        scanf("%d", &frames[i]);
    }
    int i, j, t;
    for (i = 0; i < f; i++)
    {
        if ((i + 1) % w == 0)
        {
            t = frames[i];
            send(socketfd, &t, sizeof(int), 0);
            printf("%d\t", t);
            printf("\n<< Acknowledgement of the frames sent is received by sender\n\n");
            j = 0;
        }
        else
        {
            t = frames[i];
            send(socketfd, &t, sizeof(int), 0);
            printf("%d\t", t);
        }
    }
    if (f % w != 0)
        printf("\n<< Acknowledgement of the frames sent is received by sender\n\n");

    t = -9999;

    send(socketfd, &t, sizeof(int), 0);
    close(socketfd);

    return 0;
}