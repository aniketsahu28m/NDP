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
    int sockfd, newsockfd, k;
    socklen_t actuallen;
    int retval;
    size_t leng;
    char c;
    int recedbytes, sentbytes;
    struct sockaddr_in serveraddr, clientaddr;
    char buff[max], temp[max];
    int a = 0;

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

    /* Configure server address (this socket's local address)
     * The server listens on port 3200
     * sin_addr.s_addr: Loopback address (127.0.0.1)
     */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(3200);
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    /* Configure client address (expected sender address)
     * The client sends from port 3201
     */
    clientaddr.sin_family = AF_INET;
    clientaddr.sin_port = htons(3201);
    clientaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    /* Bind the socket to the server address (assign local address to socket)
     * sockfd: Socket file descriptor
     * (struct sockaddr *)&serveraddr: Pointer to address structure
     * sizeof(serveraddr): Size of address structure
     * Returns: 0 on success, -1 on error
     */
    retval = bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

    /* Check if binding failed */
    if (retval == -1)
    {
        printf("\n Binding error");
        close(sockfd);
        exit(0);
    }

    /* Initialize actuallen with client address structure size */
    actuallen = sizeof(clientaddr);

    /* Receive and echo back characters (stop-and-wait protocol server side)
     * k < strlen(buff): Note: buff is uninitialized, this is undefined behavior
     * The loop expects characters to be sent character by character
     */
    for (k = 0; k < strlen(buff); k++)
    {
        /* Receive single character from client
         * recvfrom: Receives data and stores sender address
         * temp: Buffer to store received data
         * sizeof(temp): Maximum bytes to receive
         * 0: No special flags
         * (struct sockaddr *)&clientaddr: Output parameter for sender address
         * &actuallen: Input/output parameter for address structure size
         * Returns: Number of bytes received, -1 on error
         */
        retval = recvfrom(sockfd, temp, sizeof(temp), 0, (struct sockaddr *)&clientaddr, &actuallen);

        /* Print the received character */
        puts(temp);

        /* Check if recv failed */
        if (retval == -1)
        {
            close(sockfd);
            exit(0);
        }

        /* Echo the received character back to client
         * sendto: Sends datagram to specified address
         * Returns: Number of bytes sent, -1 on error
         */
        retval = sendto(sockfd, temp, sizeof(temp), 0, (struct sockaddr *)&clientaddr, actuallen);
        printf("\n");
    }

    /* Read input string from user (unsafe - no bounds checking)
     * Note: This appears to be unreachable code due to the for loop above
     * The loop uses strlen(buff) but buff is uninitialized at this point
     */
    gets(buff);

    /* Send the entire string to the client
     * sockfd: Socket file descriptor
     * buff: Data to send
     * sizeof(buff): Number of bytes to send
     * 0: No special flags
     * (struct sockaddr *)&clientaddr: Destination address
     * actuallen: Size of destination address structure
     * Returns: Number of bytes sent, -1 on error
     */
    retval = sendto(sockfd, buff, sizeof(buff), 0, (struct sockaddr *)&clientaddr, actuallen);

    /* Check if send failed */
    if (retval == -1)
    {
        close(sockfd);
        exit(0);
    }

    /* Close the socket to release resources */
    close(sockfd);

    return 0;
}
