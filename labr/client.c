#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>

#define N 10
#define port2 3560
#define SIZE 300
#define n 50

typedef struct
{
    int roll;
    int sem;
    int marks[5];
    char name[50];
    char gender;
} student;

typedef struct
{
    int date;
    int status;
    char method[n];
    char version[n];
    char accept[n];
    char connection[n];
    char useragent[n];
    student stu;
} request;

typedef struct
{
    int status;
    char statusmsg[n];
    char version[n];
    char datetime[n];
    char connection[n];
    char content[n];
    student stu;
} response;

int main()
{
    struct sockaddr_in server_addr;
    int cli_sock = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port2);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    int ret = connect(cli_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret < 0)
    {
        printf("error in connection\n");
        return 0;
    }
    printf("client connected to proxy server\n");
    request req = {2015, 0, "GET", "HTTP/1.1", "TEXT", "Keep-Alive", "Mozilla"};
    int reg;
    int semno;
    while (1)
    {
        printf("Enter the register number and semester number to get details and enter (-999) to stop\n");
        response resp;
        scanf("%d", &req.stu.roll);
        reg = req.stu.roll;
        if (reg == -999)
        {
            send(cli_sock, &req, sizeof(req), 0);
            break;
        }
        scanf("%d", &req.stu.sem);
        semno = req.stu.sem;
        send(cli_sock, &req, sizeof(req), 0);
        int recieve = recv(cli_sock, &resp, sizeof(resp), 0);
        if (resp.stu.roll == -1000)
        {
            printf("SERVER RESPONSE DETAILS\n");
            printf("%s\t%d\t%s\n", resp.version, resp.status, resp.statusmsg);
            printf("Date : %s", resp.datetime);
            printf("Content-type : %s\n", resp.content);
            printf("Connection : %s\n\n", resp.connection);
            printf("roll number not found\n");
            continue;
        }
        else
        {
            printf("SERVER RESPONSE DETAILS\n");
            printf("%s\t%d\t%s\n", resp.version, resp.status, resp.statusmsg);
            printf("Date : %s", resp.datetime);
            printf("Content-type : %s\n", resp.content);
            printf("Connection : %s\n\n", resp.connection);
            if (resp.stu.sem == 1)
            {
                printf("Name\trollno\tSem\tGender\tEVS\tEngineering physics\tDiscrete maths\tcomputational thinking\tC programming\n");
                printf("%s\t%d\t%d\t%c\t %d\t\t %d\t\t %d\t\t\t%d\t\t\t%d\n", resp.stu.name,
                       resp.stu.roll, resp.stu.sem, resp.stu.gender, resp.stu.marks[0],
                       resp.stu.marks[1], resp.stu.marks[2], resp.stu.marks[3], resp.stu.marks[4]);
            }
            if (resp.stu.sem == 2)
            {
                printf("Name\trollno\tSem\tGender\tEngineering graphics\tProbability and statistics\tEngineering chemistry\tApp development\tInterview skills\n");
                printf("%s\t%d\t%d\t%c\t\t%d\t\t\t%d\t\t\t\t%d\t\t\t%d\t\t  %d\n", resp.stu.name,
                       resp.stu.roll, resp.stu.sem, resp.stu.gender, resp.stu.marks[0],
                       resp.stu.marks[1], resp.stu.marks[2], resp.stu.marks[3], resp.stu.marks[4]);
            }
            if (resp.stu.sem == 3)
            {
                printf("Name\trollno\tSem\tGender\tMachnine learning\tjava\tpython\tdeep learning\toperating systems\n");
                printf("%s\t%d\t%d\t%c\t\t%d\t\t %d\t%d\t\t%d\t\t%d\n", resp.stu.name, resp.stu.roll,
                       resp.stu.sem, resp.stu.gender, resp.stu.marks[0], resp.stu.marks[1],
                       resp.stu.marks[2], resp.stu.marks[3], resp.stu.marks[4]);
            }
            printf("\n");
        }
    }
    close(cli_sock);
    printf("client disconnected from proxy server\n\n");
    return 0;
}