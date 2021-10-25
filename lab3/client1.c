#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#define SIZE 256
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

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Enter portnumber also\n");
        return 1;
    }

    int client_socket;
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    char buffer[SIZE], currTime[SIZE];
    struct sockaddr_in server_address;

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(argv[1]));
    server_address.sin_addr.s_addr = INADDR_ANY;
    int conn_status = connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address));

    if (conn_status == -1)
    {
        printf("Socket Connection Failed\n");
        exit(0);
    }
    else
    {
        time_stamp(currTime);
        printf("Socket Connection Established on %s \n", currTime);
    }

    char buff[SIZE];
    int n;
    while (1)
    {
        //first the client should send the message
        bzero(buff, SIZE);

        printf("Enter the string : ");
        n = 0;
        fgets(buff, SIZE - 1, stdin);

        write(client_socket, buff, sizeof(buff));
        // sending the written message through the socket
        if ((strncmp(buff, "exit", 4)) == 0)
        {
            time_stamp(currTime);
            printf("Client Exiting at %s...\n", currTime);
            break;
        }

        // clearing the buffer
        bzero(buff, SIZE);

        // getting the string from the server and printing
        read(client_socket, buff, sizeof(buff));
        printf("From Server : %s", buff);

        if ((strncmp(buff, "exit\n", 4)) == 0)
        {
            printf("Client Exit...\n");
            break;
        }
    }
    close(client_socket);

    return 0;
}