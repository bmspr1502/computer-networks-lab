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
    fprintf(stdout, "Opening Authentication Server at %s:%d.\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
    return socketfd;
}

int get_connected_socket(long port)
{
    int socketfd = 0, n = 0;
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

    if (connect(socketfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        fprintf(stderr, "Error in connection.\n");
        return -1;
    }
    fprintf(stdout, "Connection established with %s:%d.\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
    return socketfd;
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
    bzero(&res, sizeof(res));
    n = recv(clientfd, &req, sizeof(req), 0);
    printf("<< From client: \n");
    printReq(req);
    bzero(linebuff, sizeof(linebuff));
    i = 0, j = 0;

    FILE *fd = fopen("auth.txt", "r");
    if (fd < 0)
    {
        res.status = 502;
        strcpy(res.msg, "Database not opened. Bad gateway.");
        send(clientfd, &res, sizeof(res), 0);
        printf(">> To client: \n");
        printRes(res);
        return;
    }

    while (fgets(linebuff, sizeof(linebuff), fd))
    {
        user = strtok(linebuff, "@");
        pwd = strtok(NULL, "\n");
        if (strcmp(req.username, user) == 0 && strcmp(req.password, pwd) == 0)
        {
            request file;
            file = req;
            strcpy(file.path, "/file/getFile");
            int fileserver = get_connected_socket(FILEPORT);

            printf(">> Forwarding request to service providing server\n");
            send(fileserver, &file, sizeof(request), 0);
            response fileres;
            recv(fileserver, &fileres, sizeof(response), 0);
            printf("<< Received response from Service providing server: \n");
            printRes(fileres);

            printf(">> Sending response to client.\n");
            send(clientfd, &fileres, sizeof(response), 0);

            flag = 1;
            break;
        }
    }

    if (flag == 0)
    {
        res.status = 407;
        strcpy(res.msg, "Invalid Username/Password. Access denied.");
        send(clientfd, &res, sizeof(res), 0);
        printf(">> To client: \n");
        printRes(res);
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