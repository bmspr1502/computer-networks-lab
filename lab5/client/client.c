#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define PROXYPORT 1502
#define SIZE 2048
#define size 128

typedef struct
{
    char host[size];
    char filename[size];
    char contentType[size];
    char date[size];
} request;

typedef struct
{
    int status;
    char dest[size];
    char statusMsg[size];
    char date[size];
    char connection[size];
} response;

void printReq(request r)
{
    printf("\nRequest sent: %s\nFilename: %s\ncontent-type: %s\nTime: %s\n", r.host, r.filename, r.contentType, r.date);
}

void printRes(response s)
{
    printf("\nResponse received: \n%d - %s\nDestination: %s\nDate: %s\nconnection: %s\n", s.status, s.statusMsg, s.dest, s.date, s.connection);
}

int main()
{
    int sockfd = 0, stat = 0;
    struct sockaddr_in server_address;
    time_t t;
    char fileData[SIZE];
    bzero(fileData, SIZE);
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PROXYPORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(sockfd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        printf("\nError while connecting, exiting...\n");
        return -1;
    }

    char buffer[SIZE], filename[size];

    //ESTABLISH CONNECTION WITH THE PROXY SERVER
    fprintf(stdout, "Connection established with %s:%d.\n", inet_ntoa(server_address.sin_addr), ntohs(server_address.sin_port));
    fprintf(stdout, "Enter filename to retreive : ");
    scanf("%s", filename);

    request r;

    strcpy(r.host, "192.168.131.31");
    strcpy(r.filename, filename);
    time(&t);
    strcpy(r.date, ctime(&t));
    strcpy(r.contentType, "text/html");

    //SEND PACKET WITH REQUIRED FILENAME
    stat = send(sockfd, &r, sizeof(r), 0);
    if (stat < 0)
    {
        printf("\nError while sending filename, exiting...\n");
        return -1;
    }
    printReq(r);

    response resp;

    //GET RESPONSE FROM PROXY SERVER
    stat = recv(sockfd, &resp, sizeof(resp), 0);
    if (stat < 0)
    {
        printf("\nError while receiving data, exiting...\n");
        return -1;
    }
    printRes(resp);

    if (resp.status == 200)
    {
        //DISPLAY FILE CONTENT
        recv(sockfd, fileData, sizeof(fileData), 0);
        printf("\nContent-length: %d\nContent: \n%s", strlen(fileData), fileData);
    }

    printf("\nClosing connention.\n");
    close(sockfd);
    return 0;
}