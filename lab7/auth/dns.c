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
#define AUTHPORT 15023
extern int errno;

typedef struct
{
    char website[32];
    char from[16];
    char to[16];
} request;

typedef struct
{
    int status;
    char msg[32];
    char ipaddress[16];
    char from[16];
    char to[16];
} response;

void printReq(request r)
{
    printf("Request for: %s\nFrom: %s\nTo: %s\n\n", r.website, r.from, r.to);
}

void printRes(response r)
{
    printf("Status: %d\nMessage: %s\n", r.status, r.msg);
    if (r.status == 200)
    {
        printf("IP address: %s\n", r.ipaddress);
    }
    printf("From: %s\nTo: %s\n", r.from, r.to);
}

int communicate(int clientfd, request *req, response *res, response *clires, char *type, int PORTNO, socklen_t length, struct sockaddr_in client_addr)
{
    int sentbytes, recvbytes;
    struct sockaddr_in server_addr;

    int serverfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverfd < 0)
    {
        fprintf(stderr, "Error in socket creation.\n");
        clires->status = 503;
        strcpy(clires->msg, "Error in socket creation");
        sendto(clientfd, &clires, sizeof(clires), 0, (struct sockaddr *)&client_addr, length);
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORTNO);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    sentbytes = sendto(serverfd, req, sizeof(*req), 0, (struct sockaddr *)&server_addr, length);
    printf("\n>> Request sent to %s\n", type);
    printReq(*req);

    recvbytes = recvfrom(serverfd, res, sizeof(*res), 0, NULL, NULL);
    if (res->status != 200)
    {
        clires->status = res->status;
        strcpy(clires->msg, res->msg);
        sendto(clientfd, clires, sizeof(*clires), 0, (struct sockaddr *)&client_addr, length);
        printRes(*clires);
        return -1;
    }
    printf("<< Response from %s:\n", type);
    printRes(*res);
    fprintf(stdout, "IP for %s: %s.\n", req->website, res->ipaddress);
    close(serverfd);
    return 0;
}

int main()
{
    int socketfd = 0, clientfd = 0, sentbytes, recvbytes;
    socklen_t length = sizeof(struct sockaddr_in);
    struct sockaddr_in host_addr, client_addr;
    request req;
    response res;

    socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketfd < 0)
    {
        fprintf(stderr, "Error in socket creation.\n");
        return -1;
    }

    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(AUTHPORT);
    inet_pton(AF_INET, "127.0.0.1", &host_addr.sin_addr);

    if (bind(socketfd, (struct sockaddr *)&host_addr, sizeof(host_addr)) < 0)
    {
        fprintf(stderr, "Error in binding port to socket.\n");
        return -1;
    }

    while (1)
    {
        printf("\n====================================================\n");
        char buffer[32];
        bzero(&req, sizeof(req));
        bzero(&res, sizeof(res));

        recvbytes = recvfrom(socketfd, &req, sizeof(req), 0, (struct sockaddr *)&client_addr, &length);
        printf(">> Request received from local dns:-\n");
        printReq(req);
        strcpy(res.from, req.to);
        strcpy(res.to, req.from);

        char location[32];
        // here we can use a hashmap with key being the ip address and the record being the value
        if (strncmp(req.to, "97.23.24.12", sizeof(req.to)) == 0)
        {
            strcpy(location, "amazon.in.txt");
        }
        else if (strncmp(req.to, "15.15.15.15", sizeof(req.to)) == 0)
        {
            strcpy(location, "google.com.txt");
        }
        else
        {
            strcpy(location, "unavailable");
        }

        printf("Opening Auth DNS at %s\n", req.to);
        FILE *fd = fopen(location, "r");
        if (!fd)
        {
            res.status = 502;
            strcpy(res.msg, "Could not access Auth records.");
            sentbytes = sendto(socketfd, &res, sizeof(res), 0, (struct sockaddr *)&client_addr, length);
            printRes(res);
            continue;
        }

        strcpy(buffer, req.website);
        char linebuff[40], filebuff[400], ip[20], tempbuff[40], lastbuff[40];
        char *temp, *iptemp;
        int flag = 0, i;
        linebuff[0] = '\0';
        lastbuff[0] = '\0';
        filebuff[0] = '\0';
        ip[0] = '\0';
        while (fgets(linebuff, sizeof(linebuff), fd))
        {
            strcpy(tempbuff, linebuff);
            // separating out the part before space
            temp = strtok(tempbuff, " ");
            if (flag == 0 && strncmp(temp, buffer, strlen(temp)) == 0)
            {
                flag = 1;
                strcpy(lastbuff, linebuff);
                iptemp = strtok(NULL, "\n");
                // separating out the part after space (ipaddress) and copying to `ip`
                for (i = 0; *iptemp != '\0'; i++, iptemp++)
                    ip[i] = *iptemp;
                ip[i] = '\0';
            }
            else
            {
                strcat(filebuff, linebuff);
            }
        }
        fclose(fd);
        if (flag == 0)
        {
            res.status = 404;
            strcpy(res.msg, "Website Not Found");
            sentbytes = sendto(socketfd, &res, sizeof(res), 0, (struct sockaddr *)&client_addr, length);
            printRes(res);
        }
        else
        {
            int fdes = open(location, O_WRONLY);
            strcat(filebuff, lastbuff);
            write(fdes, filebuff, strlen(filebuff));
            close(fdes);

            res.status = 200;
            strcpy(res.msg, "Website available.");
            strcpy(res.ipaddress, ip);
            printf(">> Sending response.\n");
            sentbytes = sendto(socketfd, &res, sizeof(res), 0, (struct sockaddr *)&client_addr, length);
            printRes(res);
        }
    }
    close(socketfd);
    return 0;
}