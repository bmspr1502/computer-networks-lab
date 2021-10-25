#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include <time.h>
#define SIZE 1024

//function to print error
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    char buffer[SIZE];
    struct sockaddr_in serv_addr, cli_addr;

    int n;

    if (argc < 2)
    {
        fprintf(stderr, "No port number has been provided. Exiting..");
        exit(1);
    }

    //opening socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
        error("ERROR WHILE OPENING SOCKET\n");

    bzero((char *)&serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR WHILE BINDING\n");

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);

    if (newsockfd < 0)
    {
        error("ERROR WHILE ACCEPTING\n");
    }

    int a, b, c, x;
    char msg[SIZE];
    char *choises[5] = {"Addition", "Subtraction", "Multiplication", "Division", "Exit"};
Start:

    //asking the choice from the user
    strcpy(msg, "Menu-> \n1.Addition\n2.Subtraction\n3.Multiplication\n4.Division\n5.Exit\nEnter your choice-> ");
    n = write(newsockfd, msg, strlen(msg));
    if (n < 0)
        error("ERROR WHILE WRITING TO SOCKET");

    read(newsockfd, &x, sizeof(int));
    printf("\nClient: Operation choise is: %s\n", choises[x - 1]);

    if (x == 5)
    {
        printf("Client: Exit.\n");
        goto End;
    }

    //getting the first number
    strcpy(msg, "Enter your number 1: ");
    n = write(newsockfd, msg, strlen(msg));
    if (n < 0)
        error("ERROR WHILE WRITING TO SOCKET");

    read(newsockfd, &a, sizeof(int));
    printf("Client: First number is: %d\n", a);

    //getting the second number
    strcpy(msg, "Enter your number 2: ");
    n = write(newsockfd, msg, strlen(msg));
    if (n < 0)
        error("ERROR WHILE WRITING TO SOCKET");

    read(newsockfd, &b, sizeof(int));
    printf("Client: Second number is: %d\n", b);

    //calculating the output
    switch (x)
    {
    case 1:
        c = a + b;
        break;
    case 2:
        c = a - b;
        break;
    case 3:
        c = a * b;
        break;
    case 4:
        c = a / b;
        break;
    case 5:
        printf("Client: Exit.\n");
        goto End;
        break;
    }

    printf("Server: The answer is: %d\n", c);
    //sending the output
    write(newsockfd, &c, sizeof(int));

    if (x != 5)
        goto Start;

End:
    //closing the connection
    close(newsockfd);
    close(sockfd);
    return 0;
}