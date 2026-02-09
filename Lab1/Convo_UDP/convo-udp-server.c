#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>

#define MAXSIZE 90

int main(void)
{
    int sockfd, retval, sentbytes, recedbytes;
    socklen_t actuallen;
    struct sockaddr_in serveraddr, clientaddr;
    char buff[MAXSIZE];

    /* Create a UDP socket
     * AF_INET: IPv4 address family
     * SOCK_DGRAM: UDP (unreliable, connectionless datagrams)
     * 0: Default protocol for SOCK_DGRAM (UDP)
     * Returns: socket file descriptor on success, -1 on error
     */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    /* Check if socket creation failed */
    if (sockfd == -1)
    {
        printf("\nSocket creation error");
        return 1;
    }

    /* Configure server address structure (bind socket to this address)
     * sin_family: Address family (IPv4)
     * sin_port: Server port number in network byte order
     * sin_addr.s_addr: INADDR_ANY accepts connections on all available interfaces
     */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(3388);
    serveraddr.sin_addr.s_addr = INADDR_ANY;

    /* Configure client address structure (for tracking connected clients)
     * sin_family: Address family (IPv4)
     * sin_port: Client port number (will be set from received packets)
     * sin_addr.s_addr: Client IP address (will be set from received packets)
     */
    clientaddr.sin_family = AF_INET;
    clientaddr.sin_port = htons(3389);
    clientaddr.sin_addr.s_addr = INADDR_ANY;

    /* Bind socket to server address and port
     * sockfd: Socket file descriptor
     * (struct sockaddr*)&serveraddr: Pointer to server address structure
     * sizeof(serveraddr): Size of address structure
     * Returns: 0 on success, -1 on error
     */
    retval = bind(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));

    /* Check if bind failed */
    if (retval == -1)
    {
        printf("Binding error");
        close(sockfd);
        return 1;
    }

    /* Conversation loop - continuously receive and send datagrams */
    while (1)
    {
        /* Set size of client address structure for recvfrom */
        actuallen = sizeof(clientaddr);

        /* Receive datagram from the client
         * sockfd: Socket file descriptor
         * buff: Buffer to store received data
         * sizeof(buff): Maximum number of bytes to receive
         * 0: No special flags
         * (struct sockaddr*)&clientaddr: Pointer to store sender address information
         * &actuallen: Pointer to size of address structure (input/output)
         * Returns: Number of bytes received, -1 on error
         */
        recedbytes = recvfrom(sockfd, buff, sizeof(buff), 0, (struct sockaddr*)&clientaddr, &actuallen);

        /* Check if receive failed */
        if (recedbytes == -1)
        {
            printf("Receiving error\n");
            close(sockfd);
            return 1;
        }

        /* Print the received message */
        puts(buff);
        printf("\n");

        /* Check if client wants to stop the conversation */
        if (buff[0] == 's' && buff[1] == 't' && buff[2] == 'o' && buff[3] == 'p')
        {
            break;
        }

        /* Read response from server input (no bounds checking - unsafe) */
        scanf("%s", buff);

        /* Send datagram to the client
         * sockfd: Socket file descriptor
         * buff: Buffer containing data to send
         * sizeof(buff): Number of bytes to send
         * 0: No special flags
         * (struct sockaddr*)&clientaddr: Pointer to client address structure
         * sizeof(clientaddr): Size of address structure
         * Returns: Number of bytes sent, -1 on error
         */
        sentbytes = sendto(sockfd, buff, sizeof(buff), 0, (struct sockaddr*)&clientaddr, sizeof(clientaddr));

        /* Check if send failed */
        if (sentbytes == -1)
        {
            printf("Sending error");
            close(sockfd);
            return 1;
        }

        /* Check if server wants to stop the conversation */
        if (buff[0] == 's' && buff[1] == 't' && buff[2] == 'o' && buff[3] == 'p')
        {
            break;
        }
    }

    /* Close the socket to release resources */
    close(sockfd);

    return 0;
}
