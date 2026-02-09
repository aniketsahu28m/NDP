#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

#define MAXSIZE 1024

int main(void)
{
    int sockfd, retval;
    int recedbytes, sentbytes;
    struct sockaddr_in serveraddr;
    char buff[MAXSIZE], filename[MAXSIZE];

    /* Create a TCP socket
     * AF_INET: IPv4 address family
     * SOCK_STREAM: TCP (reliable, connection-oriented byte stream)
     * 0: Default protocol for SOCK_STREAM (TCP)
     */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1)
    {
        printf("\nSocket Creation Error");
        exit(0);
    }

    /* Configure server address structure
     * sin_family: Address family (IPv4)
     * sin_port: Port number in network byte order
     * sin_addr.s_addr: Server IP address (localhost)
     */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(3397);
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    /* Initiate connection to the server */
    retval = connect(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));

    if (retval == -1)
    {
        printf("Connection error");
        exit(0);
    }

    /* Get alphanumeric string from user */
    printf("Input string: ");
    scanf("%s", buff);

    /* Send string to server */
    sentbytes = send(sockfd, buff, sizeof(buff), 0);

    if (sentbytes == -1)
    {
        printf("Send error");
        close(sockfd);
        exit(0);
    }

    /* Receive results from server (child process result) */
    recedbytes = recv(sockfd, buff, sizeof(buff), 0);

    if (recedbytes == -1)
    {
        close(sockfd);
        exit(0);
    }

    printf("At the client side:\n");
    printf("%s\n", buff);

    /* Receive results from server (parent process result) */
    recedbytes = recv(sockfd, buff, sizeof(buff), 0);

    if (recedbytes == -1)
    {
        close(sockfd);
        exit(0);
    }

    printf("%s\n", buff);

    close(sockfd);

    return 0;
}
