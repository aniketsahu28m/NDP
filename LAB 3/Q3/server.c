#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/wait.h>


#define MAXSIZE 1024

int main(void)
{
    int sockfd, newsockfd, retval;
    socklen_t actuallen;
    int recedbytes, sentbytes;
    struct sockaddr_in serveraddr, clientaddr;
    char buff[MAXSIZE], numbers[MAXSIZE], chars[MAXSIZE];
    char result[MAXSIZE];
    pid_t pid;

    /* Create a TCP socket
     * AF_INET: IPv4 address family
     * SOCK_STREAM: TCP (reliable, connection-oriented byte stream)
     * 0: Default protocol for SOCK_STREAM (TCP)
     */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1)
    {
        printf("\nSocket creation error");
        exit(0);
    }

    /* Configure server address structure
     * sin_family: Address family (IPv4)
     * sin_port: Port number in network byte order
     * sin_addr.s_addr: Any local IP address
     */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(3397);
    serveraddr.sin_addr.s_addr = htons(INADDR_ANY);

    /* Bind the socket to the configured address and port */
    retval = bind(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));

    if (retval == -1)
    {
        printf("Binding error");
        close(sockfd);
        exit(0);
    }

    /* Mark the socket as passive (listening for incoming connections) */
    retval = listen(sockfd, 1);

    if (retval == -1)
    {
        close(sockfd);
        exit(0);
    }

    actuallen = sizeof(clientaddr);

    /* Accept an incoming connection from a client */
    newsockfd = accept(sockfd, (struct sockaddr*)&clientaddr, &actuallen);

    if (newsockfd == -1)
    {
        close(sockfd);
        exit(0);
    }

    /* Receive alphanumeric string from client */
    recedbytes = recv(newsockfd, buff, sizeof(buff), 0);

    if (recedbytes == -1)
    {
        close(sockfd);
        close(newsockfd);
        exit(0);
    }

    printf("Input string: %s\n", buff);

    /* Separate numbers and characters */
    int num_count = 0, char_count = 0;
    for (int i = 0; buff[i] != '\0'; i++)
    {
        if (isdigit(buff[i]))
        {
            numbers[num_count++] = buff[i];
        }
        else
        {
            chars[char_count++] = buff[i];
        }
    }
    numbers[num_count] = '\0';
    chars[char_count] = '\0';

    /* Create child process */
    pid = fork();

    if (pid == 0)
    {
        /* Child process - sort numbers in ascending order */
        printf("Output at the child process of the server: ");

        for (int i = 0; i < num_count - 1; i++)
        {
            for (int j = 0; j < num_count - i - 1; j++)
            {
                if (numbers[j] > numbers[j + 1])
                {
                    char temp = numbers[j];
                    numbers[j] = numbers[j + 1];
                    numbers[j + 1] = temp;
                }
            }
        }

        printf("%s\n", numbers);

        /* Send result with process ID to client */
        sprintf(result, "Child (PID %d): %s", getpid(), numbers);
        sentbytes = send(newsockfd, result, sizeof(result), 0);

        close(newsockfd);
        exit(0);
    }
    else
    {
        /* Parent process - sort characters in descending order */
        printf("Output at the parent process of the server: ");

        for (int i = 0; i < char_count - 1; i++)
        {
            for (int j = 0; j < char_count - i - 1; j++)
            {
                if (chars[j] < chars[j + 1])
                {
                    char temp = chars[j];
                    chars[j] = chars[j + 1];
                    chars[j + 1] = temp;
                }
            }
        }

        printf("%s\n", chars);

        /* Send result with process ID to client */
        sprintf(result, "Parent (PID %d): %s", getpid(), chars);
        sentbytes = send(newsockfd, result, sizeof(result), 0);

        /* Wait for child process to complete */
        wait(NULL);
    }

    close(sockfd);
    close(newsockfd);

    return 0;
}
