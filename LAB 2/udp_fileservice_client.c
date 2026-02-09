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
    int sockfd, retval;
    int recedbytes, sentbytes;
    struct sockaddr_in serveraddr, clientaddr;
    char buff[MAXSIZE], filename[100];
    socklen_t size;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1)
    {
        printf("Socket Creation Error\n");
        exit(1);
    }

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(3388);
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    clientaddr.sin_family = AF_INET;
    clientaddr.sin_port = htons(3389);
    clientaddr.sin_addr.s_addr = INADDR_ANY;

    retval = bind(sockfd, (struct sockaddr*)&clientaddr, sizeof(clientaddr));
    if (retval == -1)
    {
        printf("Binding error\n");
        close(sockfd);
        exit(1);
    }

    /* Send file name */
    printf("Enter file name: ");
    scanf("%s", filename);
    sendto(sockfd, filename, sizeof(filename), 0,
           (struct sockaddr*)&serveraddr, sizeof(serveraddr));

    /* Receive file status */
    size = sizeof(serveraddr);
    recvfrom(sockfd, buff, sizeof(buff), 0,
             (struct sockaddr*)&serveraddr, &size);
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

        sendto(sockfd, &option, sizeof(option), 0,
               (struct sockaddr*)&serveraddr, sizeof(serveraddr));

        if (option == 1)   /* Search */
        {
            char str[50];
            printf("Enter string to search: ");
            scanf("%s", str);
            sendto(sockfd, str, sizeof(str), 0,
                   (struct sockaddr*)&serveraddr, sizeof(serveraddr));

            recvfrom(sockfd, buff, sizeof(buff), 0,
                     (struct sockaddr*)&serveraddr, &size);
            printf("%s\n", buff);
        }

        else if (option == 2)  /* Replace */
        {
            char s1[50], s2[50];
            printf("Enter string to replace: ");
            scanf("%s", s1);
            printf("Enter new string: ");
            scanf("%s", s2);

            sendto(sockfd, s1, sizeof(s1), 0,
                   (struct sockaddr*)&serveraddr, sizeof(serveraddr));
            sendto(sockfd, s2, sizeof(s2), 0,
                   (struct sockaddr*)&serveraddr, sizeof(serveraddr));

            recvfrom(sockfd, buff, sizeof(buff), 0,
                     (struct sockaddr*)&serveraddr, &size);
            printf("%s\n", buff);
        }

        else if (option == 3)  /* Reorder */
        {
            recvfrom(sockfd, buff, sizeof(buff), 0,
                     (struct sockaddr*)&serveraddr, &size);
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
