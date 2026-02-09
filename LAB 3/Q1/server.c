#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

#define MAXSIZE 1024

int main(void)
{
    int sockfd, newsockfd, retval;
    socklen_t actuallen;
    int recedbytes, sentbytes;
    struct sockaddr_in serveraddr, clientaddr;
    char buff[MAXSIZE];
    pid_t pid, ppid, cpid;

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
    serveraddr.sin_port = htons(3394);
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

    /* Get parent process PID and PPID */
    pid = getpid();
    ppid = getppid();
    printf("Server Parent Process - PID: %d, PPID: %d\n", pid, ppid);

    /* Create child process for full duplex chat */
    cpid = fork();

    if (cpid == 0)
    {
        /* Child process - receive messages from client */
        printf("Server Child Process - PID: %d, PPID: %d\n", getpid(), getppid());

        while (1)
        {
            recedbytes = recv(newsockfd, buff, sizeof(buff), 0);

            if (recedbytes == -1)
            {
                close(sockfd);
                close(newsockfd);
                exit(0);
            }

            if (strcmp(buff, "exit") == 0)
            {
                break;
            }

            printf("Client: %s\n", buff);
        }

        close(newsockfd);
        exit(0);
    }
    else
    {
        /* Parent process - send messages to client */
        while (1)
        {
            printf("Server: ");
            scanf("%s", buff);

            sentbytes = send(newsockfd, buff, sizeof(buff), 0);

            if (sentbytes == -1)
            {
                close(sockfd);
                close(newsockfd);
                exit(0);
            }

            if (strcmp(buff, "exit") == 0)
            {
                break;
            }
        }

        /* Wait for child process to complete */
        wait(NULL);
    }

    close(sockfd);
    close(newsockfd);

    return 0;
}
