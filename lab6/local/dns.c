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

#define ROOTPORT 15021
#define TLDPORT 15022  // THE TLD AND AUTH ARE TO BE RECEIVED FROM ROOT AND TLD SERVERS RESPECTIVELY
#define AUTHPORT 15023 // BUT WE HARD CODE AS THIS IS A SIMULATION ONLY FOR UNDERSTANDING PURPOSES.
#define LOCALPORT 1502

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
    printf(">> Response from %s:\n", type);
    printRes(*res);
    fprintf(stdout, "TLD server IP for %s: %s.\n", req->website, res->ipaddress);
    close(serverfd);
    return 0;
}

// assuming local dns is at 8.8.8.8
int main()
{
    int clientfd = 0, localfd = 0;
    int serverfd = 0, tldfd = 0, authfd = 0;
    socklen_t length = sizeof(struct sockaddr_in);
    struct sockaddr_in host_addr, root_addr, tld_addr, auth_addr, client_addr;
    int sentbytes, recvbytes;

    clientfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientfd < 0)
    {
        fprintf(stderr, "Error in socket creation.\n");
        return -1;
    }

    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(LOCALPORT);
    inet_pton(AF_INET, "127.0.0.1", &host_addr.sin_addr);

    if (bind(clientfd, (struct sockaddr *)&host_addr, sizeof(host_addr)) < 0)
    {
        fprintf(stderr, "Error in binding port to socket.\n");
        return -1;
    }

    printf("Opening LOCAL DNS at 8.8.8.8 Using Iterative method.\n\n");

    while (1)
    {
        printf("\n====================================================\n");
        request clireq;
        response clires;
        recvbytes = recvfrom(clientfd, &clireq, sizeof(clireq), 0, (struct sockaddr *)&client_addr, &length);

        if (strncmp(clireq.website, "exit", sizeof("exit")) == 0)
            break;

        printf(">> Request from client: \n");
        printReq(clireq);
        strcpy(clires.to, clireq.from);
        strcpy(clires.from, "8.8.8.8");

        request rootreq, tldreq, authreq;
        response rootres, tldres, authres;
        strcpy(authreq.website, clireq.website);

        int i = 0, j = 0, k = 0;
        while (clireq.website[i++] != '.')
            ;
        while (clireq.website[i] != '.')
        {
            tldreq.website[j++] = clireq.website[i++];
        }
        tldreq.website[j++] = clireq.website[i++];
        while (clireq.website[i] != ' ' && clireq.website[i] != '\0')
        {
            tldreq.website[j++] = clireq.website[i];
            rootreq.website[k++] = clireq.website[i];
            i++;
        }
        tldreq.website[j] = '\0';
        rootreq.website[k] = '\0';

        // ROOT DNS
        strcpy(rootreq.from, "8.8.8.8");
        strcpy(rootreq.to, "10.10.10.10");
        if (communicate(clientfd, &rootreq, &rootres, &clires, "ROOTDNS", ROOTPORT, length, client_addr) == -1)
        {
            continue;
        }

        // TLD DNS
        strcpy(tldreq.from, "8.8.8.8");
        strcpy(tldreq.to, rootres.ipaddress);
        if (communicate(clientfd, &tldreq, &tldres, &clires, "TLD", TLDPORT, length, client_addr) == -1)
        {
            continue;
        }

        // Authoritative DNS
        strcpy(authreq.from, "8.8.8.8");
        strcpy(authreq.to, tldres.ipaddress);
        if (communicate(clientfd, &authreq, &authres, &clires, "authDNS", AUTHPORT, length, client_addr) == -1)
        {
            continue;
        }

        printf("Sending Response back to client:-");
        clires.status = authres.status;
        strcpy(clires.msg, authres.msg);
        strcpy(clires.ipaddress, authres.ipaddress);
        sendto(clientfd, &clires, sizeof(clires), 0, (struct sockaddr *)&client_addr, length);
    }

    close(clientfd);

    return 0;
}