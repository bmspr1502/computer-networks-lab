#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVERPORT 1502
#define FILEPORT 3555

typedef struct
{
    char filename[32];
    char username[32];
    char password[32];
} request;

typedef struct
{
    int status;
    char msg[32];
    long PORTNO;
} response;

void printReq(request *r)
{
    printf("\nRequest for: %s\nUsername: %s\nPassword: %s\n", r->filename, r->username, r->password);
}

void printRes(response *r)
{
    printf("\nStatus: %d\n Message: %s\n", r->status, r->msg);
    if (r->status == 200)
        printf("Port No.: %ld\n", r->PORTNO);
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

void file_transfer(int client)
{
    int socketfd, clientfd, length = 0;
    struct sockaddr_in server_addr, client_addr;
    char buffer[512], filename[32];
    response res;

    socketfd = listening_socket_connection(FILEPORT);
    if (socketfd < 0)
        return;

    res.status = 200;
    strcpy(res.msg, "Sign in successful.");
    res.PORTNO = FILEPORT;

    printf(">> Sending to client: ");
    printRes(&res);
    send(client, &res, sizeof(res), 0);

    clientfd = accept(socketfd, (struct sockaddr *)&client_addr, &length);
    if (clientfd < 0)
    {
        fprintf(stderr, "Error in accepting connection.\n");
        close(socketfd);
        return;
    }

    fprintf(stdout, "New file trasfer connection established.\n");
    while (1)
    {
        response newres;
        recv(clientfd, filename, sizeof(filename), 0);
        printf("<< Request for filename: %s", filename);

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
            printRes(&newres);
            continue;
        }
        else
        {
            newres.status = 200;
            strcpy(newres.msg, "Requested file is found.");
            newres.PORTNO = FILEPORT;
            send(clientfd, &newres, sizeof(newres), 0);
            printf("\n>> To client: \n");
            printRes(&newres);

            fprintf(stdout, "Waiting confirmation.\n");

            recv(clientfd, buffer, sizeof(buffer), 0);
            if (strncmp(buffer, "ABORT", strlen("ABORT")) == 0)
            {
                fprintf(stdout, "Client aborted the transfer.\n");
                continue;
            }

            read(fd, buffer, sizeof(buffer));
            send(clientfd, buffer, strlen(buffer) + 1, 0);
            fprintf(stdout, "Confirmation received and file sent.\n");
        }
    }
    close(clientfd);
    close(socketfd);
    fprintf(stdout, "Closing the file-transfer connection.\n\n");
    return;
}

void authorize_and_handle(int clientfd)
{
    request req;
    response res;
    int n, i, j, flag = 0;
    char linebuff[512];
    char *user, *pwd;

AUTH:
    bzero(&req, sizeof(req));
    n = recv(clientfd, &req, sizeof(req), 0);
    printf("<< From client: \n");
    printReq(&req);
    if (strcmp(req.filename, "req_handshake") != 0)
    {
        res.status = 403;
        strcpy(res.msg, "Invalid request. Try again.");
        send(clientfd, &res, sizeof(res), 0);
        printf(">> To client: \n");
        printRes(&res);
        goto AUTH;
    }
    bzero(linebuff, sizeof(linebuff));
    i = 0, j = 0;

    FILE *fd = fopen("auth.txt", "r");
    if (fd < 0)
    {
        res.status = 502;
        strcpy(res.msg, "Database not opened. Bad gateway.");
        send(clientfd, &res, sizeof(res), 0);
        printf(">> To client: \n");
        printRes(&res);
        return;
    }

    while (fgets(linebuff, sizeof(linebuff), fd))
    {
        user = strtok(linebuff, "@");
        pwd = strtok(NULL, "\n");
        if (strcmp(req.username, user) == 0 && strcmp(req.password, pwd) == 0)
        {
            file_transfer(clientfd);
            flag = 1;
            break;
        }
    }

    if (flag == 0)
    {
        res.status = 507;
        strcpy(res.msg, "Invalid Username/Password. Access denied.");
        send(clientfd, &res, sizeof(res), 0);
        printf(">> To client: \n");
        printRes(&res);
        goto AUTH;
    }

    return;
}

int main()
{
    int socketfd, clientfd, length = 0;
    struct sockaddr_in client_addr;

    socketfd = listening_socket_connection(SERVERPORT);
    if (socketfd < 0)
    {
        return -1;
    }

    while (1)
    {
        clientfd = accept(socketfd, (struct sockaddr *)&client_addr, &length);
        if (clientfd < 0)
        {
            fprintf(stderr, "Error in accepting connection.\n");
            continue;
        }
        fprintf(stdout, "New control connection established.\n");
        authorize_and_handle(clientfd);
        close(clientfd);
    }
    close(socketfd);
    return 0;
}