#include <string.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>

#define max 50

int main(void)
{
    int sockfd, retval, newsockfd, k = 0;
    socklen_t actuallen;
    char c;
    char temp[3];
    size_t leng;
    FILE *fp;
    int recedbytes, sentbytes;
    struct sockaddr_in serveraddr, clientaddr;
    char buff[max];
    char temp1[max];

    /* Create a UDP socket
     * AF_INET: IPv4 address family
     * SOCK_DGRAM: UDP (connectionless, datagram-based)
     * 0: Default protocol for SOCK_DGRAM (UDP)
     * Returns: socket file descriptor on success, -1 on error
     */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    /* Check if socket creation failed */
    if (sockfd == -1)
    {
        printf("\n Socket creation error");
        exit(0);
    }

    /* Configure client address (this socket's local address)
     * The client uses port 3201
     * sin_addr.s_addr: Loopback address (127.0.0.1)
     */
    clientaddr.sin_family = AF_INET;
    clientaddr.sin_port = htons(3201);
    clientaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    /* Configure server address (destination address)
     * The server listens on port 3200
     */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(3200);
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    /* Bind the socket to the client address (assign local address to socket)
     * sockfd: Socket file descriptor
     * (struct sockaddr *)&clientaddr: Pointer to address structure
     * sizeof(clientaddr): Size of address structure
     * Returns: 0 on success, -1 on error
     */
    retval = bind(sockfd, (struct sockaddr *)&clientaddr, sizeof(clientaddr));

    /* Check if binding failed */
    if (retval == -1)
    {
        printf("\n Binding error");
        close(sockfd);
        exit(0);
    }

    /* Read input string from user (unsafe - no bounds checking) */
    gets(buff);

    /* Initialize actuallen with server address structure size */
    actuallen = sizeof(serveraddr);

    /* Send each character individually and wait for acknowledgment
     * This implements a simple stop-and-wait protocol character by character
     * k <= strlen(buff): Include null terminator in transmission
     */
    for (k = 0; k <= strlen(buff); k++)
    {
        /* Extract single character into temp buffer */
        temp[0] = buff[k];
        temp[1] = '\0';

        /* Copy to temp1 for transmission */
        strcpy(temp1, temp);

        /* Send single character to server via UDP
         * sockfd: Socket file descriptor
         * temp1: Data to send (single character)
         * sizeof(temp): Size of data
         * 0: No special flags
         * (struct sockaddr *)&serveraddr: Destination address
         * actuallen: Size of destination address structure
         * Returns: Number of bytes sent, -1 on error
         */
        retval = sendto(sockfd, temp1, sizeof(temp), 0, (struct sockaddr *)&serveraddr, actuallen);

        /* Check if send failed */
        if (retval == -1)
        {
            close(sockfd);
            exit(0);
        }

        /* Update actuallen before each recvfrom call */
        actuallen = sizeof(serveraddr);

        /* Receive acknowledgment from server
         * recvfrom: Receives data and stores sender address
         * Returns: Number of bytes received, -1 on error
         */
        retval = recvfrom(sockfd, temp1, sizeof(temp1), 0, (struct sockaddr *)&serveraddr, &actuallen);

        /* Print the echoed character */
        puts(temp1);

        /* Check if recv failed */
        if (retval == -1)
        {
            close(sockfd);
            exit(0);
        }
    }

    /* Close the socket to release resources */
    close(sockfd);

    return 0;
}
