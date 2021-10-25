#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#define N 7
#define SIZE 300
#define x 50
#define port1 35600
typedef struct
{
    int roll;
    int sem;
    int marks[5];
    char name[x];
    char gender;
} student;
typedef struct
{
    int date;
    int status;
    char method[x];
    char version[x];
    char accept[x];
    char connection[x];
    char useragent[x];
    student stu;
} request;
typedef struct
{
    int status;
    char statusmsg[x];
    char version[x];
    char datetime[x];
    char content[x];
    char connection[x];
    student stu;
} response;
int n = 0;
student stud[SIZE];
int date = 2021;
int main()
{
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        printf("error in socket creation\n");
        return 0;
    }
    printf("socket created\n");
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = ntohs(port1);
    int ret = bind(server_socket, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if (ret < 0)
    {
        printf("error in binding\n");
        return 0;
    }
    printf("binding successful\n");
    printf("ENTER NUMBER OF STUDENTS TO TYPE DETAILS(1-STUDENT FOR 3 SEMESTERS)\n");
    scanf("%d", &n);
    n = 3 * n;
    for (int i = 0; i < n; i++)
    {
        printf("Enter roll no: ");
        scanf("%d", &stud[i].roll);
        printf("Enter semester number: ");
        scanf("%d", &stud[i].sem);
        printf("Enter name: ");
        scanf("%s", stud[i].name);
        printf("Enter gender(f or m): ");
        getchar();
        scanf("%c", &stud[i].gender);
        printf("Enter 5  marks\n");
        if (stud[i].sem == 1)
        {
            printf("Environmental Science : ");
            scanf("%d", &stud[i].marks[0]);
            printf("Engineering Physics    : ");
            scanf("%d", &stud[i].marks[1]);
            printf("Discrete Mathematics   : ");
            scanf("%d", &stud[i].marks[2]);
            printf("Computational thinking: ");
            scanf("%d", &stud[i].marks[3]);
            printf("C programming         : ");
            scanf("%d", &stud[i].marks[4]);
        }
        if (stud[i].sem == 2)
        {
            printf("Engineering graphics      : ");
            scanf("%d", &stud[i].marks[0]);
            printf("Probability and statistics: ");
            scanf("%d", &stud[i].marks[1]);
            printf("Engineering chemistry     : ");
            scanf("%d", &stud[i].marks[2]);
            printf("App development practice  : ");
            scanf("%d", &stud[i].marks[3]);
            printf("Interview skills          : ");
            scanf("%d", &stud[i].marks[4]);
        }
        if (stud[i].sem == 3)
        {
            printf("Machine learning  : ");
            scanf("%d", &stud[i].marks[0]);
            printf("java programming  : ");
            scanf("%d", &stud[i].marks[1]);
            printf("python programming: ");
            scanf("%d", &stud[i].marks[2]);
            printf("Deep learning     : ");
            scanf("%d", &stud[i].marks[3]);
            printf("Operating Systems : ");
            scanf("%d", &stud[i].marks[4]);
        }
        printf("\n");
    }
    if (listen(server_socket, N) < 0)
    {
        printf("error in listening\n");
        return 0;
    }
    printf("listening to port %d\n\n", port1);
    struct sockaddr_in proxy_addr;
    int proxy_len;
    int cliproxy_sock = accept(server_socket, (struct sockaddr *)&proxy_addr, &proxy_len);
    int check = 0;
    while (1)
    {
        request req;
        recv(cliproxy_sock, &req, sizeof(req), 0);
        if (check == 0)
        {
            printf("REQUEST RECEIVED FROM PROXY SERVER \n\n");
            printf("%s\t%s\n", req.method, req.version);
            printf("useragent - %s\n", req.useragent);
            printf("accept - %s\n", req.accept);
            printf("connection - %s\n", req.connection);
            printf("\n");
            check = 1;
        }
        time_t t = time(NULL);
        struct tm *tm = localtime(&t);
        response resp = {200, "ok", "HTTP/1.1"};
        strcpy(resp.datetime, asctime(tm));
        strcpy(resp.connection, "keep-alive");
        strcpy(resp.content, "Text/Html");
        int reg = req.stu.roll;
        int semno = req.stu.sem;
        int flag = 0;
        if (req.status == 0)
        {
            for (int i = 0; i < n; i++)
            {
                if (stud[i].roll == reg && stud[i].sem == semno)
                {
                    flag = 1;
                    resp.stu = stud[i];
                    send(cliproxy_sock, &resp, sizeof(resp), 0);
                    break;
                }
            }
            if (flag == 0)
            {
                resp.stu.roll = -1000;
                resp.status = 404;
                send(cliproxy_sock, &resp, sizeof(resp), 0);
            }
        }
        if (req.status == 1)
        {
            if (req.date < date)
            {
                for (int i = 0; i < n; i++)
                {
                    if (stud[i].roll == reg && stud[i].sem == semno)
                    {
                        flag = 1;
                        resp.status = 200;
                        resp.stu = stud[i];
                        send(cliproxy_sock, &resp, sizeof(resp), 0);
                        break;
                    }
                }
            }
            else
            {
                resp.status = 100;
                send(cliproxy_sock, &resp, sizeof(resp), 0);
            }
        }
    }
    close(cliproxy_sock);
    return 0;
}