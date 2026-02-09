#include<stdio.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
#include<stdlib.h>

#define MAXSIZE 1024

int main()
{
    char buff[MAXSIZE], filename[100];
    int sockfd, retval;
    struct sockaddr_in serveraddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("Socket Creation Error\n");
        exit(1);
    }

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(3388);
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    retval = connect(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if (retval == -1)
    {
        printf("Connection error\n");
        close(sockfd);
        exit(1);
    }

    /* Send file name */
    printf("Enter file name: ");
    scanf("%s", filename);
    send(sockfd, filename, sizeof(filename), 0);

    /* Receive file status */
    recv(sockfd, buff, sizeof(buff), 0);
    printf("%s\n", buff);

    if (strcmp(buff, "File not present") == 0)
    {
        close(sockfd);
        exit(0);
    }

    while (1)
    {
        int option;
        printf("\n1.Search  2.Replace  3.Reorder  4.Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &option);

        send(sockfd, &option, sizeof(option), 0);

        if (option == 1)   /* Search */
        {
            char str[50];
            printf("Enter string to search: ");
            scanf("%s", str);
            send(sockfd, str, sizeof(str), 0);

            recv(sockfd, buff, sizeof(buff), 0);
            printf("%s\n", buff);
        }

        else if (option == 2)  /* Replace */
        {
            char s1[50], s2[50];
            printf("Enter string to replace: ");
            scanf("%s", s1);
            printf("Enter new string: ");
            scanf("%s", s2);

            send(sockfd, s1, sizeof(s1), 0);
            send(sockfd, s2, sizeof(s2), 0);

            recv(sockfd, buff, sizeof(buff), 0);
            printf("%s\n", buff);
        }

        else if (option == 3)  /* Reorder */
        {
            recv(sockfd, buff, sizeof(buff), 0);
            printf("%s\n", buff);
        }

        else if (option == 4)  /* Exit */
        {
            break;
        }
    }

    close(sockfd);
    return 0;
}

