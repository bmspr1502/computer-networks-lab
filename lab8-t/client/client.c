#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define SERVERPORT 15021

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

int main()
{
    int socketfd;
    struct sockaddr_in server_addr;

    socketfd = get_connected_socket(SERVERPORT);
    request req;
    response res;
    char username[32], password[32], filename[32];

CRED:
    bzero(username, sizeof(username));
    bzero(password, sizeof(password));
    printf(">> Enter Username: ");
    scanf("%s", username);
    printf(">> Enter password: ");
    scanf("%s", password);
    printf(">> Enter required filename: ");
    scanf("%s", filename);
    strcpy(req.username, username);
    strcpy(req.password, password);
    strcpy(req.filename, filename);
    strcpy(req.host, "14.139.161.31 (cs.annauniv.edu)");
    strcpy(req.path, "/auth/login");

    printf(">> Sending to server@%d\n", SERVERPORT);
    printReq(req);
    send(socketfd, &req, sizeof(req), 0);

    recv(socketfd, &res, sizeof(res), 0);
    printf("<< received from server@%d\n", SERVERPORT);
    printRes(res);

    close(socketfd);
    return 0;
}