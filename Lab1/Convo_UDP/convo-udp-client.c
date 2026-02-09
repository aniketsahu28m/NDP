#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define MAXSIZE 50

int main(void)
{
    int sockfd, retval, sentbytes, recedbytes;
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
        printf("\nSocket Creation Error");
        return 1;
    }

    /* Configure server address structure (destination for outgoing packets)
     * sin_family: Address family (IPv4)
     * sin_port: Server port number in network byte order
     * sin_addr.s_addr: Server IP address (127.0.0.1 = localhost)
     */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(3388);
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    /* Configure client address structure (bind socket to this address)
     * sin_family: Address family (IPv4)
     * sin_port: Client port number in network byte order
     * sin_addr.s_addr: Client IP address (127.0.0.1 = localhost)
     */
    clientaddr.sin_family = AF_INET;
    clientaddr.sin_port = htons(3389);
    clientaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    /* Bind socket to client address and port
     * sockfd: Socket file descriptor
     * (struct sockaddr*)&clientaddr: Pointer to client address structure
     * sizeof(clientaddr): Size of address structure
     * Returns: 0 on success, -1 on error
     */
    retval = bind(sockfd, (struct sockaddr*)&clientaddr, sizeof(clientaddr));

    /* Check if bind failed */
    if (retval == -1)
    {
        printf("Binding error");
        close(sockfd);
        return 1;
    }

    /* Conversation loop - continuously send and receive datagrams */
    while (1)
    {
        /* Prompt user for input */
        printf("enter the text\n");

        /* Read string from user input (no bounds checking - unsafe) */
        scanf("%s", buff);

        /* Send datagram to the server
         * sockfd: Socket file descriptor
         * buff: Buffer containing data to send
         * sizeof(buff): Number of bytes to send
         * 0: No special flags
         * (struct sockaddr*)&serveraddr: Pointer to server address structure
         * sizeof(serveraddr): Size of address structure
         * Returns: Number of bytes sent, -1 on error
         */
        sentbytes = sendto(sockfd, buff, sizeof(buff), 0, (struct sockaddr*)&serveraddr, sizeof(serveraddr));

        /* Check if send failed */
        if (sentbytes == -1)
        {
            printf("Sending error");
            close(sockfd);
            return 1;
        }

        /* Check if user wants to stop the conversation */
        if (buff[0] == 's' && buff[1] == 't' && buff[2] == 'o' && buff[3] == 'p')
        {
            break;
        }

        /* Receive datagram from the server
         * sockfd: Socket file descriptor
         * buff: Buffer to store received data
         * sizeof(buff): Maximum number of bytes to receive
         * 0: No special flags
         * (struct sockaddr*)&serveraddr: Pointer to store sender address information
         * &serveraddr_size: Pointer to size of address structure (socklen_t* type required)
         * Returns: Number of bytes received, -1 on error
         */
        socklen_t serveraddr_size = sizeof(serveraddr);
        recedbytes = recvfrom(sockfd, buff, sizeof(buff), 0, (struct sockaddr*)&serveraddr, &serveraddr_size);

        /* Check if receive failed */
        if (recedbytes == -1)
        {
            printf("Receiving error");
            close(sockfd);
            return 1;
        }

        /* Print the received message */
        puts(buff);
        printf("\n");

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
