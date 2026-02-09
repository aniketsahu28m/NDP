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
    int sockfd, newsockfd, retval;
    socklen_t actuallen;
    struct sockaddr_in serveraddr, clientaddr;
    char buff[MAXSIZE], filename[100];
    FILE *fp;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("Socket creation error\n");
        exit(1);
    }

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(3388);
    serveraddr.sin_addr.s_addr = INADDR_ANY;

    retval = bind(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if (retval == -1)
    {
        printf("Binding error\n");
        close(sockfd);
        exit(1);
    }

    listen(sockfd, 1);
    printf("TCP Server waiting...\n");

    actuallen = sizeof(clientaddr);
    newsockfd = accept(sockfd, (struct sockaddr*)&clientaddr, &actuallen);
    if (newsockfd == -1)
    {
        close(sockfd);
        exit(1);
    }

    /* Receive file name */
    recv(newsockfd, filename, sizeof(filename), 0);
    fp = fopen(filename, "r");

    if (fp == NULL)
    {
        strcpy(buff, "File not present");
        send(newsockfd, buff, sizeof(buff), 0);
        close(newsockfd);
        close(sockfd);
        exit(0);
    }
    else
    {
        strcpy(buff, "File present");
        send(newsockfd, buff, sizeof(buff), 0);
    }

    while (1)
    {
        int option;
        recv(newsockfd, &option, sizeof(option), 0);

        if (option == 1)   /* Search */
        {
            char str[50], word[50];
            int count = 0;

            recv(newsockfd, str, sizeof(str), 0);
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

            send(newsockfd, buff, sizeof(buff), 0);
        }

        else if (option == 2)  /* Replace */
        {
            char s1[50], s2[50], word[50];
            int found = 0;

            recv(newsockfd, s1, sizeof(s1), 0);
            recv(newsockfd, s2, sizeof(s2), 0);

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

            send(newsockfd, buff, sizeof(buff), 0);
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
            send(newsockfd, buff, sizeof(buff), 0);
        }

        else if (option == 4)  /* Exit */
        {
            break;
        }
    }

    fclose(fp);
    close(newsockfd);
    close(sockfd);
    return 0;
}

