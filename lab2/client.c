#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include <netdb.h>
#include <time.h>
#define SIZE 1024
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void time_stamp(char *actual)
{
    bzero((char *)actual, SIZE);
    char *timestamp = (char *)malloc(sizeof(char) * SIZE);
    time_t ltime;
    ltime = time(NULL);
    struct tm *tm;
    tm = localtime(&ltime);

    sprintf(timestamp, "%02d - %02d - %04d, %02d : %02d : %02d\n\n", tm->tm_mday, tm->tm_mon + 1,
            tm->tm_year + 1900, tm->tm_hour, tm->tm_min, tm->tm_sec);
    strcpy(actual, timestamp);
    bzero((char *)timestamp, SIZE);
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno;
    char buffer[SIZE], currtime[SIZE];
    struct hostent *server;
    struct sockaddr_in serv_addr;

    int n;
    if (argc < 2)
    {
        fprintf(stderr, "No port number has been provided. Exiting..");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    portno = atoi(argv[1]);
    if (sockfd < 0)
        error("ERROR WHILE OPENING SOCKET\n");

    bzero((char *)&serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    time_stamp(currtime);
    printf("Client Connected at %s", currtime);

    int a, b, c, x;
    char msg[SIZE];
    char *choices[5] = {"Addition", "Subtraction", "Multiplication", "Division", "Exit"};
Start:
    bzero(msg, SIZE);
    n = read(sockfd, msg, SIZE);
    if (n < 0)
        error("ERROR WHILE READING FROM SOCKET");

    printf("Server: %s", msg);
    scanf("%d", &x);
    write(sockfd, &x, sizeof(int));
    printf("\nClient: The choice chosen is %s\n", choices[x - 1]);

    if (x == 5)
        goto End;

    bzero(msg, SIZE);
    n = read(sockfd, msg, SIZE);
    if (n < 0)
        error("ERROR WHILE READING FROM SOCKET");

    printf("Server: %s\nClient:", msg);
    scanf("%d", &a);
    write(sockfd, &a, sizeof(int));

    bzero(msg, SIZE);
    n = read(sockfd, msg, SIZE);
    if (n < 0)
        error("ERROR WHILE READING FROM SOCKET");

    printf("Server: %s\nClient:", msg);
    scanf("%d", &b);
    write(sockfd, &b, sizeof(int));

    read(sockfd, &c, sizeof(int));
    printf("Server: The answer is: %d\n\n", c);

    if (x != 5)
        goto Start;
End:
    time_stamp(currtime);
    printf("\nClient exiting at %s\n", currtime);
    close(sockfd);
    return 0;
}