#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define LOCALDNS 15020

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

// assuming client is at 14.139.161.31

int main()
{
    int sockfd = 0;
    struct sockaddr_in host_addr;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        fprintf(stderr, "Error in socket creation.\n");
        return -1;
    }

    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(LOCALDNS);
    inet_pton(AF_INET, "127.0.0.1", &host_addr.sin_addr);

    int stat;
    request req;
    response resp;
    printf("Opening Client at 14.139.161.31\n\n");
    while (1)
    {
        fprintf(stdout, ">> Enter the HostName: ");
        scanf("%s", req.website);
        strcpy(req.from, "14.139.161.31");
        strcpy(req.to, "8.8.8.8");
        printf("Sending Request:\n");
        stat = sendto(sockfd, &req, sizeof(req), 0, (struct sockaddr *)&host_addr, sizeof(host_addr));
        if (stat < 0)
        {
            printf("\nError while sending filename, exiting...\n");
            return -1;
        }
        printReq(req);

        if (strncmp(req.website, "exit", strlen("exit")) == 0)
            return 0;

        stat = recvfrom(sockfd, &resp, sizeof(resp), 0, NULL, NULL);
        if (stat < 0)
        {
            printf("\nError while receiving data, exiting...\n");
            return -1;
        }
        printf("<< Receiving information: ");
        printRes(resp);

        if (resp.status == 200)
            printf("\nThe website %s is available at %s.", req.website, resp.ipaddress);
        printf("\n====================================================\n");
    }

    return 0;
}