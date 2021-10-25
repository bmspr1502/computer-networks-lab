#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>

#define SIZE 256
int PORT;

void time_stamp(char *actual)
{
    bzero((char *)actual, SIZE);
    char *timestamp = (char *)malloc(sizeof(char) * SIZE);
    time_t ltime;
    ltime = time(NULL);
    struct tm *tm;
    tm = localtime(&ltime);

    sprintf(timestamp, "%02d/%02d/%04d, %02d:%02d:%02d", tm->tm_mday, tm->tm_mon + 1,
            tm->tm_year + 1900, tm->tm_hour, tm->tm_min, tm->tm_sec);
    strcpy(actual, timestamp);
    bzero((char *)timestamp, SIZE);
}

int main()
{
    int clientSocket;
    struct sockaddr_in server_address;
    char msg[SIZE], buf[SIZE], currTime[SIZE];

    printf("\nEnter the port number you want to bind to: ");
    scanf("%d", &PORT);
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0)
    {
        perror("Can't create Socket..\n");
        return 1;
    }

    printf("Client socket created..\n");
    memset(&server_address, '\0', sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;
    int c = connect(clientSocket, (struct sockaddr *)&server_address, sizeof(server_address));

    if (c < 0)
    {
        perror("Can't connect to the server..\n");
        return 1;
    }

    time_stamp(currTime);
    printf("%s\nConnected to the server..\n", currTime);

    while (1)
    {
        printf("Enter Client: ");
        fgets(buf, SIZE - 1, stdin);
        send(clientSocket, buf, sizeof(buf), 0);
        if (strcmp(buf, "exit\n") == 0)
        {
            close(clientSocket);
            time_stamp(currTime);
            printf("%s\nDisconnected from the server..\n", currTime);
            return 1;
        }
        if (recv(clientSocket, msg, sizeof(msg), 0) < 0)
        {
            perror("Can't receive message..\n");
            return 1;
        }

        printf("Server: %s", msg);
        bzero(msg, sizeof(msg));
    }

    return 0;
}