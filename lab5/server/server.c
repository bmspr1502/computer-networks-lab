#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVERPORT 35555

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

typedef struct
{
    char fileName[size];
    char modified[size];
} fileDet;
time_t t;

void printReq(request r, char *verb)
{
    printf("\nRequest %s: %s\nFilename: %s\ncontent-type: %s\nTime: %s\n", verb, r.host, r.filename, r.contentType, r.date);
}

void printRes(response s, char *verb)
{
    printf("\nResponse %s: \n%d - %s\nDestination: %s\nDate: %s\nconnection: %s\n", verb, s.status, s.statusMsg, s.dest, s.date, s.connection);
}

void sendFile(int clientfd, char filename[], char lastmod[], char clientLoc[])
{
    char fileContent[SIZE];
    bzero(fileContent, SIZE);
    response res;
    char line[100];
    int n = 0;
    struct stat file_info;
    FILE *fd = fopen(filename, "r");

    if (fd == NULL)
    {
        res.status = 404;
        strcpy(res.statusMsg, "File not found in server.");
        time(&t);
        strcpy(res.date, ctime(&t));
        strcpy(res.connection, "close");
        strcpy(res.dest, clientLoc);
        if (!(send(clientfd, &res, sizeof(res), 0) < 0))
        {
            printRes(res, "sent");
        }
        return;
    }

    //STORE THE CONTENT OF THE FILE IN FILECONTENT
    while (fgets(line, sizeof(line), fd))
    {
        strcat(fileContent, line);
    }
    fclose(fd);

    stat(filename, &file_info);
    res.status = 200;
    strcpy(res.statusMsg, "Sending new or modified file to proxy.");
    strcpy(res.date, ctime(&file_info.st_mtime));
    strcpy(res.connection, "close");
    strcpy(res.dest, clientLoc);
    fprintf(stdout, "Sending new or modified file to proxy.\n");

    //SEND THE RESPONSE MESSAGE TO THE PROXY SERVER
    if (!(send(clientfd, &res, sizeof(res), 0) < 0))
    {
        printRes(res, "sent");
    }

    //AND THEN SEND THE CONTENT
    send(clientfd, fileContent, strlen(fileContent), 0);
    return;
}

void handleConnection(int clientfd)
{
    request req;
    response res;
    struct stat file_info;
    char clientLoc[size];

    //GET REQUEST FROM PROXY SERVER
    recv(clientfd, &req, sizeof(req), 0);
    strcpy(clientLoc, req.host);

    printReq(req, "received");

    int n = stat(req.filename, &file_info);
    if (n < 0)
    {
        //IF FILE NOT FOUND, SEND 404 ERROR
        res.status = 404;
        strcpy(res.statusMsg, "File Not Found in SERVER");
        time(&t);
        strcpy(res.date, ctime(&t));
        strcpy(res.dest, clientLoc);
        strcpy(res.connection, "close");

        if (!(send(clientfd, &res, sizeof(res), 0) < 0))
        {
            printRes(res, "sent");
        }
        return;
    }
    char lastMod[size];
    strcpy(lastMod, ctime(&file_info.st_mtim));

    if (strncmp(lastMod, req.date, 24) == 0)
    {
        //IF THE LAST MODIFIED DATE CORRESPONDS WITH THE CACHE, SEND UPTODATE
        fprintf(stdout, "File up-to-date in cache.\n");

        res.status = 200;
        strcpy(res.statusMsg, "uptodate");
        strcpy(res.date, req.date);
        strcpy(res.connection, "close");
        if (!(send(clientfd, &res, sizeof(res), 0) < 0))
        {
            printRes(res, "sent");
        }
    }
    else
    {
        //SEND THE FILE FROM THE SERVER
        sendFile(clientfd, req.filename, lastMod, clientLoc);
    }
}

int main()
{
    int proxfd, servfd, length;
    struct sockaddr_in host_addr, client_addr;
    proxfd = socket(AF_INET, SOCK_STREAM, 0);
    if (proxfd < 0)
    {
        fprintf(stderr, "Error in socket creation.\n");
        return -1;
    }

    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(SERVERPORT);
    host_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(proxfd, (struct sockaddr *)&host_addr, sizeof(host_addr)) < 0)
    {
        fprintf(stderr, "Error in binding port.\n");
        return -1;
    }

    if (listen(proxfd, 5) < 0)
    {
        fprintf(stderr, "Error in listening on port.\n");
        return -1;
    }
    fprintf(stdout, "SERVER STARTED\nListening on %s:%d.\n\n", inet_ntoa(host_addr.sin_addr), ntohs(host_addr.sin_port));

    while (1)
    {
        servfd = accept(proxfd, (struct sockaddr *)&client_addr, &length);
        if (servfd < 0)
        {
            fprintf(stderr, "Error in accepting connection.\n");
            return -1;
        }
        fprintf(stdout, "New Connection established.\n");
        handleConnection(servfd);
        close(servfd);
    }

    close(proxfd);
    return 0;
}