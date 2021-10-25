#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define MAXSIZE 1024

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Enter the port number and the string to send\n");
        return 1;
    }

    int sock = 0, valread;
    struct sockaddr_in serv_addr;

    int PORT = atoi(argv[1]);
    char *msg = argv[2];

    char buffer[MAXSIZE] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Converting addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }
    send(sock, msg, strlen(msg), 0);
    printf("Your message sent has been sent to server\n");

    valread = read(sock, buffer, MAXSIZE);
    printf("%s\n", buffer);

    return 0;
}
