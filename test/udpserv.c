#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
int main()
{
    int sockfd, op1, op2, result;
    char operator;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) > 0)
    {
        printf("Socket created successfully\n");
    }
    struct sockaddr_in servaddr, clientaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = 9632;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    socklen_t length = sizeof(clientaddr);
    if ((bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) == 0)
    {
        printf("Bind successful\n");
    }

    int sin_size = sizeof(struct sockaddr_in);

    recvfrom(sockfd, &operator, sizeof(operator), 0, (struct sockaddr *)&clientaddr, &length);
    recvfrom(sockfd, &op1, sizeof(op1), 0, (struct sockaddr *)&clientaddr, &length);
    recvfrom(sockfd, &op2, sizeof(op2), 0, (struct sockaddr *)&clientaddr, &length);
    printf("\n%c, %d, %d\n", operator, op1, op2);
    switch (operator)
    {
    case '+':
        result = op1 + op2;
        printf("Addition: %d\n", result);
        break;
    case '-':
        result = op1 - op2;
        printf("Addition: %d\n", result);
        break;
    default:
        printf("Unsupported operation\n");
    }
    sendto(sockfd, &result, sizeof(result), 0, (struct sockaddr *)&clientaddr, length);
    close(sockfd);
    return 0;
}