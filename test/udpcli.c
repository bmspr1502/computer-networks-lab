#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
int main()
{
    int sockfd, op1, op2, result;
    char operator;
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = 9632;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) > 0)
    {
        printf("Socket created successfully\n");
    }

    printf("Enter the operation: ");
    scanf("%c", &operator);
    printf("Enter the operator 1: ");
    scanf("%d", &op1);
    printf("Enter the operator 2: ");
    scanf("%d", &op2);
    sendto(sockfd, &operator, sizeof(operator), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
    sendto(sockfd, &op1, sizeof(op1), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
    sendto(sockfd, &op2, sizeof(op2), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
    recvfrom(sockfd, &result, sizeof(result), 0, NULL, NULL);
    printf("Result: %d\n", result);
    close(sockfd);
    return 0;
}
