#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>

#define MAXSIZE 1024

int main()
{
    int sockfd, retval;
    socklen_t actuallen;
    struct sockaddr_in serveraddr, clientaddr;
    char buff[MAXSIZE], filename[100];
    FILE *fp;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1)
    {
        printf("Socket creation error\n");
        exit(1);
    }

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(3388);
    serveraddr.sin_addr.s_addr =htons (INADDR_ANY);

    retval = bind(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if (retval == -1)
    {
        printf("Binding error\n");
        close(sockfd);
        exit(1);
    }

    printf("UDP Server waiting...\n");

    /* Receive file name */
    actuallen = sizeof(clientaddr);
    recvfrom(sockfd, filename, sizeof(filename), 0,
             (struct sockaddr*)&clientaddr, &actuallen);

    fp = fopen(filename, "r");
    if (fp == NULL)
    {
        strcpy(buff, "File not present");
        sendto(sockfd, buff, sizeof(buff), 0,
               (struct sockaddr*)&clientaddr, actuallen);
        close(sockfd);
        exit(0);
    }
    else
    {
        strcpy(buff, "File present");
        sendto(sockfd, buff, sizeof(buff), 0,
               (struct sockaddr*)&clientaddr, actuallen);
    }

    while (1)
    {
        int option;
        recvfrom(sockfd, &option, sizeof(option), 0,
                 (struct sockaddr*)&clientaddr, &actuallen);

        if (option == 1)   /* Search */
        {
            char str[50], word[50];
            int count = 0;

            recvfrom(sockfd, str, sizeof(str), 0,
                     (struct sockaddr*)&clientaddr, &actuallen);

            rewind(fp);
            while (fscanf(fp, "%s", word) != EOF)
            {
                if (strcmp(word, str) == 0)
                    count++;
            }

            if (count > 0)
                sprintf(buff, "String found %d times", count);
            else
                strcpy(buff, "String not found");

            sendto(sockfd, buff, sizeof(buff), 0,
                   (struct sockaddr*)&clientaddr, actuallen);
        }

        else if (option == 2)  /* Replace */
        {
            char s1[50], s2[50], word[50];
            int found = 0;

            recvfrom(sockfd, s1, sizeof(s1), 0,
                     (struct sockaddr*)&clientaddr, &actuallen);
            recvfrom(sockfd, s2, sizeof(s2), 0,
                     (struct sockaddr*)&clientaddr, &actuallen);

            FILE *ft = fopen("temp.txt", "w");
            rewind(fp);

            while (fscanf(fp, "%s", word) != EOF)
            {
                if (strcmp(word, s1) == 0)
                {
                    fprintf(ft, "%s ", s2);
                    found = 1;
                }
                else
                    fprintf(ft, "%s ", word);
            }

            fclose(fp);
            fclose(ft);

            remove(filename);
            rename("temp.txt", filename);
            fp = fopen(filename, "r");

            if (found)
                strcpy(buff, "String replaced");
            else
                strcpy(buff, "String not found");

            sendto(sockfd, buff, sizeof(buff), 0,
                   (struct sockaddr*)&clientaddr, actuallen);
        }

        else if (option == 3)  /* Reorder */
        {
            char text[MAXSIZE], ch;
            int i = 0, x, y;

            rewind(fp);
            while ((ch = fgetc(fp)) != EOF)
                text[i++] = ch;

            for (x = 0; x < i - 1; x++)
            {
                for (y = x + 1; y < i; y++)
                {
                    if (text[x] > text[y])
                    {
                        char temp = text[x];
                        text[x] = text[y];
                        text[y] = temp;
                    }
                }
            }

            fp = fopen(filename, "w");
            fwrite(text, 1, i, fp);
            fclose(fp);
            fp = fopen(filename, "r");

            strcpy(buff, "File reordered");
            sendto(sockfd, buff, sizeof(buff), 0,
                   (struct sockaddr*)&clientaddr, actuallen);
        }

        else if (option == 4)  /* Exit */
        {
            break;
        }
    }

    fclose(fp);
    close(sockfd);
    return 0;
}
