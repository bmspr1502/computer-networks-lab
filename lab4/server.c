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
    const char *header = "Server: Apache/2.4.6 (CentOS) OpenSSL/1.0.2k-fips PHP/5.4.16\nLocation: https://cs.annauniv.edu \nKeep-Alive: timeout=5, max=100\nConnection: Keep-Alive\nContent-Type: text/html\n";

    pid_t cp;
    char buff[SIZE], request[SIZE], response[SIZE];
    time_t clock;
    while (1)
    {
        client_socket = accept(server_socket, NULL, NULL);
        if (client_socket < 0)
        {
            exit(0);
        }

        printf("Socket Connection Established with Client %d\n", client_ind);

        if ((cp = fork()) == 0)
        {
            while (1)
            {
                //clearing previous message
                bzero(request, SIZE);

                // read the message from client and copy it in buffer
                read(client_socket, request, sizeof(request));

                // if msg contains "DELETE" then server exit and chat ended.
                printf("\nFrom Client %d: %s\n", client_ind, request);
                if (strncmp("DELETE", request, 6) == 0)
                {
                    printf("\nConnection from Client %d terminated. Exit...\n", client_ind);
                    close(client_socket);
                    break;
                }

                // print buffer which contains the message from the client
                printf("\n\nEnter your response to client %d: ", client_ind);

                bzero(buff, SIZE);
                bzero(response, SIZE);
                fgets(buff, SIZE - 1, stdin);

                //formatting the output
                strcat(response, "HTTP/1.1 200 OK\n");
                strcat(response, "Date: ");
                time(&clock);
                strcat(response, asctime(localtime(&clock)));
                strcat(response, header);
                strcat(response, "Content-Length: ");
                int x = strlen(buff);
                char length[10];
                snprintf(length, 10, "%d\n", x);
                strcat(response, length);
                strcat(response, "<html><body>");
                strcat(response, buff);
                strcat(response, "</body><html>\n\n");
                // and send that buffer to client
                write(client_socket, response, sizeof(response));
            }
        }
        client_ind++;
    }
    return 0;
}