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
    socklen_t actuallen;
    int recedbytes, sentbytes;
    struct sockaddr_in serveraddr, clientaddr;
    char buff[MAXSIZE];

    /* Create a UDP socket
     * AF_INET: IPv4 address family
     * SOCK_DGRAM: UDP (connectionless, datagram-based)
     * 0: Default protocol for SOCK_DGRAM (UDP)
     */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd == -1)
    {
        printf("\nSocket Creation Error");
        exit(0);
    }

    /* Configure client address structure
     * sin_family: Address family (IPv4)
     * sin_port: Port number in network byte order
     * sin_addr.s_addr: Local IP address
     */
    clientaddr.sin_family = AF_INET;
    clientaddr.sin_port = htons(6769);
    clientaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    /* Configure server address structure */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(6767);
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    /* Bind the socket to the client address */
    retval = bind(sockfd, (struct sockaddr*)&clientaddr, sizeof(clientaddr));

    if (retval == -1)
    {
        printf("Binding error");
        close(sockfd);
        exit(0);
    }

    /* Get process information */
    printf("Client PID: %d, PPID: %d\n", getpid(), getppid());

    actuallen = sizeof(serveraddr);

    /* Get string from user */
    printf("Enter string for permutation: ");
    scanf("%s", buff);

    /* Send string to server */
    sentbytes = sendto(sockfd, buff, sizeof(buff), 0, (struct sockaddr*)&serveraddr, actuallen);

    if (sentbytes == -1)
    {
        printf("Send error");
        close(sockfd);
        exit(0);
    }

    /* Receive response from server */
    recedbytes = recvfrom(sockfd, buff, sizeof(buff), 0, (struct sockaddr*)&serveraddr, &actuallen);

    if (recedbytes == -1)
    {
        close(sockfd);
        exit(0);
    }

    printf("%s\n", buff);
    printf("Check server output for permutations\n");

    close(sockfd);

    return 0;
}
