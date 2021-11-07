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

    // MAKING CONNECTION WITH THE SERVER
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

    printf("<<<Connection has been established.>>>\n");
    float rtt, delay;

    char input[128];

    // GETTING INPUT FOR THE RTT AND DELAY, TO CALCULATE THE TOTAL TIME
    printf(">> Enter The Round Trip Time : ");
    scanf("%f", &rtt);

    printf(">> Enter The Uniform Delay : ");
    scanf("%f", &delay);

    printf(">> Enter Message you want to send: ");
    scanf("%s", input);

    int value = (int)(rtt / delay), trips = 0;
    float totalRTT = 0;

    send(socketfd, &value, sizeof(int), 0);

    int i = 0, packet = 1;
    while (i < strlen(input))
    {
        int j, temp;
        char msg[2], receivebuffer[10];

        printf(">> Sending : ");
        for (j = 0; j < packet && i < strlen(input); j++, i++)
        {
            msg[0] = input[i];
            msg[1] = '\0';
            printf("%s", msg);
            send(socketfd, msg, sizeof(msg), 0);
        }
        if (i >= strlen(input))
            send(socketfd, "$", strlen("$") + 1, 0);
        printf("\n>> Stream sent. %d packets.\n\n", j);

        packet = value;
        char acknowledgement[10];

        temp = sprintf(acknowledgement, "ACK %d", j);
        acknowledgement[temp] = '\0';
        recv(socketfd, receivebuffer, sizeof(receivebuffer), 0);
        if (strncmp(receivebuffer, acknowledgement, strlen(acknowledgement)) == 0)
        {
            printf("<< Acknowledgement has been received for %d packets.\n\n", atoi(&receivebuffer[4]));
            totalRTT += rtt;
            trips += 1;
        }
    }

    printf("Total Round Trips : %d\n Total Time : %f.\n", trips, totalRTT);
    return 0;
}
