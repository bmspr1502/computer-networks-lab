#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#define N 7
#define SIZE 300
#define port1 35600
#define port2 3560
#define x 50
typedef struct
{
    int roll;
    int sem;
    int marks[5];
    char name[x];
    char gender;
} student;
typedef struct
{
    int date;
    int status;
    char method[x];
    char version[x];
    char accept[x];
    char connection[x];
    char useragent[x];
    student stu;
} request;
typedef struct
{
    int status;
    char statusmsg[x];
    char version[x];
    char datetime[x];
    char connection[x];
    char content[x];
    student stu;
} response;

int main()
{
    struct sockaddr_in serveraddr;
    struct sockaddr_in cli_addr;
    int clilen;
    int proxy_sock;
    int k = 0;
    student cache[SIZE];
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        printf("error in socket creation\n");
        return 0;
    }
    printf("socket created\n");
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = ntohs(port2);
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    int ret = bind(server_socket, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if (ret < 0)
    {
        printf("error in binding\n");
        return 0;
    }
    printf("binding succesful\n");
    if (listen(server_socket, N) < 0)
    {
        printf("error in listening\n");
        return 0;
    }
    printf("listening to port %d\n\n", port2);
    int cli_sock = accept(server_socket, (struct sockaddr *)&cli_addr, &clilen);
    request req;
    struct sockaddr_in proxyaddr;
    proxy_sock = socket(AF_INET, SOCK_STREAM, 0);
    proxyaddr.sin_family = AF_INET;
    proxyaddr.sin_port = htons(port1);
    proxyaddr.sin_addr.s_addr = INADDR_ANY;
    int status = connect(proxy_sock, (struct sockaddr *)&proxyaddr, sizeof(proxyaddr));
    if (status < 0)
    {
        printf("error in connection\n");
        return 0;
    }
    int check = 0;
    int y = 0;
    while (1)
    {
        request req;
        int recieve = recv(cli_sock, &req, sizeof(req), 0);
        req.date = 2020;
        if (check == 0)
        {
            printf("Client Request Headers\n");
            printf("%s\t%s\n", req.method, req.version);
            printf("Accept - %s\n", req.accept);
            printf("Connection - %s\n", req.connection);
            printf("useragent - %s\n", req.useragent);
            printf("\n");
            check = 1;
        }
        int flag = 0;
        time_t t = time(NULL);
        struct tm *tm = localtime(&t);
        response resp = {200, "ok", "HTTP/1.1"};
        strcpy(resp.datetime, asctime(tm));
        strcpy(resp.connection, "Keep-Alive");
        strcpy(resp.content, "Text/Html");
        if (req.stu.roll == -999)
        {
            break;
        }
        for (int i = 0; i < k; i++)
        {
            if (req.stu.roll == cache[i].roll && req.stu.sem == cache[i].sem)
            {
                flag = 1;
                req.status = 1;
                send(proxy_sock, &req, sizeof(req), 0);
                recv(proxy_sock, &resp, sizeof(resp), 0);
                if (resp.status == 100)
                {
                    printf("PROXY SERVER HAS THE UPDATED DATA\n");
                    resp.stu = cache[i];
                    send(cli_sock, &resp, sizeof(resp), 0);
                }
                break;
            }
        }
        if (flag == 1 && resp.status == 200)
        {
            printf("\nPROXY GETS THE DATA FROM ORIGIN SERVER TO GET THE UPDATED DATA\n");
            send(cli_sock, &resp, sizeof(resp), 0);
        }
        if (flag == 0)
        {
            printf("Cache Miss!\n");
            printf("PROXY SERVER SENDING REQUEST TO ORIGIN SERVER\n");
            send(proxy_sock, &req, sizeof(req), 0);
            recv(proxy_sock, &resp, sizeof(resp), 0);
            printf("RESPONSE RECEIVED FROM ORIGIN SERVER\n\n");
            if (y == 0)
            {
                printf("Origin server response headers\n");
                printf("%d\t%s\t%s\n", resp.status, resp.statusmsg, resp.version);
                printf("Connection - %s\n", resp.connection);
                printf("Content-type - %s\n", resp.content);
                printf("Date - %s\n", resp.datetime);
                y = 1;
            }
            cache[k] = resp.stu;
            k++;
            send(cli_sock, &resp, sizeof(resp), 0);
        }
    }
    close(proxy_sock);
    close(server_socket);
    return 0;
}