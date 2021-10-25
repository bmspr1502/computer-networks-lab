#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Enter portnumber also\n");
        return 1;
    }

    int client_socket;
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    char buffer[256];
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
        time_t t;
        time(&t);
        printf("Socket Connection Established on %s \n", ctime(&t));
    }

    int a, b;
    printf("Enter any 2 numbers \n");
    scanf("%d", &a);
    scanf("%d", &b);

    //sending the server the data written to the client
    write(client_socket, &a, sizeof(a));
    write(client_socket, &b, sizeof(b));

    int add, sub, div, mul;
    //getting the result from the server
    read(client_socket, &add, sizeof(add));
    read(client_socket, &sub, sizeof(sub));
    read(client_socket, &mul, sizeof(mul));
    read(client_socket, &div, sizeof(div));

    //displaying the result
    printf("The Result from the Server is : \n");
    printf("Addition : %d\n", add);
    printf("Subtraction : %d\n", sub);
    printf("Division : %d\n", div);
    printf("Multiplication : %d \n", mul);

    close(client_socket);

    return 0;
}