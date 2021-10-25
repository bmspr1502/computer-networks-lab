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
    int server_socket;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_address;

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(argv[1]));
    server_address.sin_addr.s_addr = INADDR_ANY;

    bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address));
    listen(server_socket, 3);
    int client_socket, client_ind = 1;
    struct sockaddr_in newAddr;

    pid_t cp;
    char currTime[SIZE];
    while (1)
    {
        client_socket = accept(server_socket, NULL, NULL);
        if (client_socket < 0)
        {
            exit(0);
        }
        time_stamp(currTime);
        printf("Socket Connection Established with Client %d at %s \n", client_ind, currTime);

        if ((cp = fork()) == 0)
        {
            int a, b, mul, add, div, sub;
            close(server_socket);

            read(client_socket, &a, sizeof(a));
            read(client_socket, &b, sizeof(b));
            printf("Client %d: a = %d, b = %d\n", client_ind, a, b);

            add = a + b;
            sub = a - b;
            mul = a * b;
            div = a / b;

            printf("The Result of Arithmetic Operations for Client %d are : \n", client_ind);
            printf("Addition : %d \n", add);
            printf("Subtraction : %d \n", sub);
            printf("Multiplication : %d \n", mul);
            printf("Division : %d \n", div);

            printf("\n");
            write(client_socket, &add, sizeof(add));
            write(client_socket, &sub, sizeof(sub));
            write(client_socket, &mul, sizeof(mul));
            write(client_socket, &div, sizeof(div));
        }
        client_ind++;
    }
    return 0;
}