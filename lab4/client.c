#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define SIZE 1024

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Enter portnumber also\n");
        return 1;
    }

    int client_socket;
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_address;
    time_t clock;

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
        printf("Socket Connection Established\n");
    }

    char *headers = "HTTP/1.1 \nHost: www.annauniv.edu\nConnection: close\nUser-Agent: SSH-terminal\nAccept: text/html\nAccept-Language: en-US,en;\n";
    char buff[SIZE], request[SIZE], response[SIZE];
    while (1)
    {
        //first the client should send the message
        bzero(buff, SIZE);

        printf("\nEnter the string : ");
        fgets(buff, SIZE - 1, stdin);
        if ((strncmp(buff, "exit", 4)) == 0)
        {
            strcat(request, "DELETE /connection\n");
        }
        else
        {
            strcat(request, "GET /msg=?");
            strcat(request, buff);
        }
        strcat(request, headers);

        // sending the written message through the socket
        write(client_socket, request, sizeof(request));
        if ((strncmp(buff, "exit", 4)) == 0)
        {
            printf("Client Exiting..\n");
            break;
        }
        // clearing the buffer
        bzero(buff, SIZE);
        bzero(request, SIZE);

        // getting the string from the server and printing
        read(client_socket, response, sizeof(response));
        printf("\nFrom Server : \n%s", response);
        bzero(response, SIZE);
    }
    close(client_socket);

    return 0;
}