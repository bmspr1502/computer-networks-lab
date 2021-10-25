#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PROXYPORT 1502
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

void handleConnection(int clientfd)
{
    request clireq, proxreq;
    response proxres, servres;
    int socketfd = 0;
    struct sockaddr_in server_addr;
    char fileName[size], clientLoc[size];

    recv(clientfd, &clireq, sizeof(clireq), 0);
    printReq(clireq, "received");

    strcpy(fileName, clireq.filename);
    strcpy(clientLoc, clireq.host);

    //CREATE A CONNECTION WITH THE SERVER
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd < 0)
    {
        fprintf(stderr, "Error in socket creation.\n");
        proxres.status = 503;
        strcpy(proxres.dest, clientLoc);
        strcpy(proxres.statusMsg, "Error while opening socket");
        time(&t);
        strcpy(proxres.date, ctime(&t));
        strcpy(proxres.connection, "close");

        if (!(send(clientfd, &proxres, sizeof(proxres), 0)) < 0)
        {
            printRes(proxres, "sent");
        }
        return;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVERPORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (connect(socketfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        fprintf(stderr, "Error in connecting to server.\n");
        proxres.status = 504;
        strcpy(proxres.dest, clientLoc);
        strcpy(proxres.statusMsg, "Error while connecting to server");
        time(&t);
        strcpy(proxres.date, ctime(&t));
        strcpy(proxres.connection, "close");
        if (!(send(clientfd, &proxres, sizeof(proxres), 0)) < 0)
        {
            printRes(proxres, "sent");
        }
        return;
    }

    printf("Connected to server\n");

    fileDet f, temp, store[5];
    strcpy(f.fileName, fileName);
    char fileContent[SIZE];
    bzero(fileContent, SIZE);

    FILE *log_file = NULL, *wlog_file = NULL, *fd = NULL;
    int newlog_flag = 0;
    char filebuffer[512], linebuffer[512];

    // TRY TO OPEN THE FILE IN THE PROXY SERVER'S FILESYSTEM (WEB CACHE)
    log_file = fopen(fileName, "r");
    int loc = 0;
    if (log_file == NULL) //THE FILE IS NOT FOUND IN THE WEB CACHE
    {
        printf("File is not found in Proxy cache.\n Sending request to Server.\n");

        strcpy(proxreq.host, "14.139.161.31");
        strcpy(proxreq.filename, fileName);
        strcpy(proxreq.contentType, "text/html");
        time(&t);
        strcpy(proxreq.date, ctime(&t));
        printReq(proxreq, "sent");

        //SEND A REQUEST TO SERVER WITH THE DESIRED FILE
        send(socketfd, &proxreq, sizeof(proxreq), 0);

        //RECEIVED MESSAGE
        recv(socketfd, &servres, sizeof(servres), 0);
        printRes(servres, "received");
        strcpy(f.modified, servres.date);

        if (servres.status != 200)
        {
            fprintf(stderr, "%s\n", servres.statusMsg);
            send(clientfd, &servres, sizeof(servres), 0);
            return;
        }
        else
        {
            //RECEIVED FILE CONTENT
            bzero(fileContent, SIZE);
            recv(socketfd, fileContent, sizeof(fileContent), 0);
            printf("\nContent: %s\n", fileContent);
        }
    }
    else //IN THE CASE WHERE THE FILE IS ALREADY PRESENT IN THE CACHE
    {
        fd = fopen("fileLog.dat", "r");
        /* THE LOGS ARE STORED IN THE FORM OF STRUCTURES AND HENCE IS NOT HUMAN READABLE */
        while (fread(&temp, sizeof(temp), 1, fd))
        {
            loc++;
            if (strcmp(temp.fileName, fileName) == 0) //IF THE FILENAME MATCHES
            {
                strcpy(proxreq.host, "14.139.161.31");
                strcpy(proxreq.contentType, "text/html");
                strcpy(proxreq.filename, fileName);
                strcpy(proxreq.date, temp.modified);
                printReq(proxreq, "sent");

                //SEND A REQUEST WITH THE DATE MODIFIED OF THE CACHE
                send(socketfd, &proxreq, sizeof(proxreq), 0);

                recv(socketfd, &servres, sizeof(servres), 0);
                printRes(servres, "received");

                //IF THE CACHE'S VERSION IS UP-TO-DATE
                if (strncmp(servres.statusMsg, "uptodate", 8) == 0)
                {
                    printf("\nThe cache is uptodate.\n");
                    bzero(fileContent, SIZE);
                    while (fgets(linebuffer, sizeof(linebuffer), log_file))
                    {
                        strcat(fileContent, linebuffer);
                    }

                    proxres.status = 200;
                    strcpy(proxres.statusMsg, "OK cached data");
                    strcpy(proxres.dest, clientLoc);
                    strcpy(proxres.date, temp.modified);
                    strcpy(proxres.connection, "close");

                    //SEND THE FILE FROM THE PROXY SERVER'S VERSION
                    if (send(clientfd, &proxres, sizeof(proxres), 0) >= 0)
                    {
                        printRes(proxres, "sent");
                        printf("Sending File content: %s\n", fileContent);
                        send(clientfd, fileContent, strlen(fileContent), 0);
                    }
                    return;
                }
                else //WHEN CACHE IS NOT UPTODATE
                {
                    newlog_flag = 1;
                    fprintf(stdout, "File outdated in CACHE\n");
                    strcpy(f.modified, servres.date);
                    strcpy(f.fileName, fileName);

                    //GET UPDATED CONTENT FROM THE SERVER
                    bzero(fileContent, SIZE);
                    recv(socketfd, fileContent, sizeof(fileContent), 0);
                    fprintf(stdout, "New data: %s %s ", f.modified, fileContent);
                    continue;
                }
            }
        }
        fclose(fd);
    }

    wlog_file = fopen("fileLog.dat", "a");
    if (newlog_flag) //IN THE CASE OF UPDATING DATA
    {
        //LOC CONTAINS THE LOCATION OF THE DESIRED STRUCTURE IN THE FILE
        fseek(wlog_file, (loc - 1) * sizeof(fileDet), SEEK_SET);
    }

    //WRITE THE DATA
    fwrite(&f, sizeof(f), 1, wlog_file);
    fclose(wlog_file);

    fd = fopen(fileName, "w");
    fputs(fileContent, fd);
    fclose(fd);

    proxres.status = 200;
    strcpy(proxres.statusMsg, "OK Fetched from Server");
    strcpy(proxres.date, f.modified);
    strcpy(proxres.connection, "close");
    strcpy(proxres.dest, clientLoc);

    //SEND THE DATA TO THE CLIENT
    if (!(send(clientfd, &proxres, sizeof(proxres), 0) < 0))
    {
        printRes(proxres, "sent");
        send(clientfd, fileContent, strlen(fileContent), 0);
    }
    return;
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
    host_addr.sin_port = htons(PROXYPORT);
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
    fprintf(stdout, "Listening on %s:%d.\n", inet_ntoa(host_addr.sin_addr), ntohs(host_addr.sin_port));

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