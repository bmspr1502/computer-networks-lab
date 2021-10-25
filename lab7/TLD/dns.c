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
#define TLDPORT 15022
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
    close(serverfd);
    return 0;
}

int main()
{
    int socketfd = 0, clientfd = 0, sentbytes, recvbytes;
    socklen_t length = sizeof(struct sockaddr_in);
    struct sockaddr_in host_addr, client_addr;

    socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketfd < 0)
    {
        fprintf(stderr, "Error in socket creation.\n");
        return -1;
    }

    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(TLDPORT);
    inet_pton(AF_INET, "127.0.0.1", &host_addr.sin_addr);

    if (bind(socketfd, (struct sockaddr *)&host_addr, sizeof(host_addr)) < 0)
    {
        fprintf(stderr, "Error in binding port to socket.\n");
        return -1;
    }

    while (1)
    {
        printf("\n====================================================\n");
        request req, authreq;
        response res, authres;
        recvbytes = recvfrom(socketfd, &req, sizeof(req), 0, (struct sockaddr *)&client_addr, &length);
        printf(">> Request received from local dns:-\n");
        printReq(req);
        strcpy(res.from, req.to);
        strcpy(res.to, req.from);

        char buffer[32], domain[32];
        int i = 0, j = 0;
        strcpy(buffer, req.website);
        while (buffer[i++] != '.')
            ;

        while (buffer[i] != '\0')
            domain[j++] = buffer[i++];
        domain[j] = '\0';
        char location[32];

        printf("Opening TLD at %s\n", req.to);
        // here we can use a hashmap with key being the ip address and the record being the value
        if (strncmp(req.to, "11.11.11.11", sizeof(req.to)) == 0)
        {
            strcpy(location, "com.txt");
        }
        else if (strncmp(req.to, "6.6.6.6", sizeof(req.to)) == 0)
        {
            strcpy(location, "in.txt");
        }
        else
        {
            strcpy(location, "unavailable");
        }

        FILE *fd = fopen(location, "r");
        if (!fd)
        {
            res.status = 502;
            strcpy(res.msg, "Could not open required TLD records.");
            sentbytes = sendto(socketfd, &res, sizeof(res), 0, (struct sockaddr *)&client_addr, length);
            printRes(res);
            continue;
        }
        char linebuff[40], filebuff[400], ip[20], tempbuff[40], lastbuff[40];
        char *temp, *iptemp;
        int flag = 0;
        linebuff[0] = '\0';
        lastbuff[0] = '\0';
        filebuff[0] = '\0';
        ip[0] = '\0';
        while (fgets(linebuff, sizeof(linebuff), fd))
        {
            strcpy(tempbuff, linebuff);
            temp = strtok(tempbuff, " ");
            if (flag == 0 && strncmp(temp, domain, strlen(temp)) == 0)
            {
                flag = 1;
                strcpy(lastbuff, linebuff);
                iptemp = strtok(NULL, "\n");
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
            res.status = 405;
            strcpy(res.msg, "Auth Record Not Found");
            sentbytes = sendto(socketfd, &res, sizeof(res), 0, (struct sockaddr *)&client_addr, length);
            printRes(res);
        }
        else
        {
            int fdes = open(location, O_WRONLY);
            strcat(filebuff, lastbuff);
            write(fdes, filebuff, strlen(filebuff));
            close(fdes);
            printf("\nIP for AUTH SERVER of %s: %s.\n", domain, ip);
        }

        printf(">> Sending query to TLD server");

        // TLD DNS
        strcpy(authreq.from, req.to);
        strcpy(authreq.to, ip);
        strcpy(authreq.website, req.website);
        if (communicate(socketfd, &authreq, &authres, &res, "AUTH DNS", AUTHPORT, length, client_addr) == -1)
        {
            continue;
        }

        res.status = authres.status;
        strcpy(res.msg, authres.msg);
        strcpy(res.ipaddress, authres.ipaddress);
        printf("\n>> Sending response back:-\n");
        sentbytes = sendto(socketfd, &res, sizeof(res), 0, (struct sockaddr *)&client_addr, length);
        printRes(res);
    }
    close(socketfd);
    return 0;
}