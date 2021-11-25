#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVERPORT 15021
#define FILEPORT 35550

typedef struct
{
    char filename[32];
    char username[32];
    char password[32];
    char path[32];
    char host[32];
} request;

typedef struct
{
    int status;
    char msg[32];
    char fileContent[512];
} response;

void printReq(request r)
{
    printf("GET %s HTTP1.1\n", r.path);
    printf("User Agent: XFCE-Terminal(cent-os)\n");
    printf("Host: %s\n", r.host);
    printf("Connection: Keep-Alive\n\n");
    printf("userName=%s&passWord=%s&fileName=%s\n", r.username, r.password, r.filename);
}

void printRes(response r)
{
    printf("HTTP1.1 %d %s\n", r.status, r.msg);
    printf("Server: CMD/2.2.14 (cent-os)\n");
    printf("Connection: Closed\n\n");
    if (r.status == 200)
    {
        printf("Content: \n%s\n", r.fileContent);
    }
}

int listening_socket_connection(long port)
{
    int socketfd;
    struct sockaddr_in server_addr;

    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd < 0)
    {
        fprintf(stderr, "Error in socket creation.\n");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if (bind(socketfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        fprintf(stderr, "Error in binding socket to port.\n");
        return -1;
    }

    if (listen(socketfd, 5) < 0)
    {
        fprintf(stderr, "Error in listening in %s:%d.\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
        return -1;
    }
    fprintf(stderr, "Listening at %s:%d.\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
    return socketfd;
}

int main()
{
    int socketfd, clientfd, length = 0;
    struct sockaddr_in server_addr, client_addr;
    char buffer[512], filename[32];
    request req;

    socketfd = listening_socket_connection(FILEPORT);
    if (socketfd < 0)
        return -1;

    while (1)
    {
        clientfd = accept(socketfd, (struct sockaddr *)&client_addr, &length);
        if (clientfd < 0)
        {
            fprintf(stderr, "Error in accepting connection.\n");
            close(socketfd);
            return -1;
        }

        fprintf(stdout, "New file trasfer connection established.\n");
        char filename[32];
        response newres;

        bzero(&req, sizeof(req));
        bzero(&newres, sizeof(newres));

        recv(clientfd, &req, sizeof(req), 0);
        printf("<< Request from client: \n");
        printReq(req);

        strcpy(filename, req.filename);

        if (strncmp(filename, "exit", strlen("exit")) == 0)
        {
            fprintf(stdout, "Client exiting file_transfer.\n");
            break;
        }

        int fd = open(filename, O_RDONLY);
        if (fd < 0)
        {
            newres.status = 404;
            strcpy(newres.msg, "Requested file not found.");
            send(clientfd, &newres, sizeof(newres), 0);
            printf("\n>> To client: \n");
            printRes(newres);
            continue;
        }
        else
        {
            newres.status = 200;
            strcpy(newres.msg, "Requested file is found.");
            read(fd, buffer, sizeof(buffer));
            strcpy(newres.fileContent, buffer);
            send(clientfd, &newres, sizeof(newres), 0);
            fprintf(stdout, ">> Confirmation received and file sent.\n");
            printRes(newres);
        }
        close(clientfd);
    }

    close(socketfd);
    fprintf(stdout, "<< Closing the file-transfer connection. >>\n\n");

    return 0;
}