#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>

#define MAXSIZE 90

int main(void)
{
    int sockfd, newsockfd, retval, sentbytes, recedbytes;
    socklen_t actuallen;
    struct sockaddr_in serveraddr, clientaddr;
    char buff[MAXSIZE];

    /* Create a TCP socket
     * AF_INET: IPv4 address family
     * SOCK_STREAM: TCP (reliable, connection-oriented byte stream)
     * 0: Default protocol for SOCK_STREAM (TCP)
     * Returns: socket file descriptor on success, -1 on error
     */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    /* Check if socket creation failed */
    if (sockfd == -1)
    {
        printf("\nSocket creation error");
        return 1;
    }

    /* Configure server address structure
     * sin_family: Address family (IPv4)
     * sin_port: Port number in network byte order (htons converts host to network short)
     * sin_addr.s_addr: INADDR_ANY accepts connections on all available interfaces
     */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(3388);
    serveraddr.sin_addr.s_addr = INADDR_ANY;

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

    /* Mark socket as passive, ready to accept incoming connections
     * sockfd: Socket file descriptor
     * 1: Maximum number of pending connections in the queue
     * Returns: 0 on success, -1 on error
     */
    retval = listen(sockfd, 1);

    /* Check if listen failed */
    if (retval == -1)
    {
        printf("Listen error");
        close(sockfd);
        return 1;
    }

    /* Wait for and accept an incoming connection
     * sockfd: Socket file descriptor
     * (struct sockaddr*)&clientaddr: Pointer to store client address information
     * &actuallen: Pointer to size of client address structure (input/output)
     * Returns: New socket file descriptor for the connection, -1 on error
     */
    actuallen = sizeof(clientaddr);
    newsockfd = accept(sockfd, (struct sockaddr*)&clientaddr, &actuallen);

    /* Check if accept failed */
    if (newsockfd == -1)
    {
        printf("Accept error");
        close(sockfd);
        return 1;
    }

    /* Conversation loop - continuously receive and send messages */
    while (1)
    {
        /* Clear buffer to receive new message */
        memset(buff, '\0', sizeof(buff));

        /* Receive message from the client
         * newsockfd: Socket file descriptor for the client connection
         * buff: Buffer to store received data
         * sizeof(buff): Maximum number of bytes to receive
         * 0: No special flags
         * Returns: Number of bytes received (0 if connection closed), -1 on error
         */
        recedbytes = recv(newsockfd, buff, sizeof(buff), 0);

        /* Check if receive failed */
        if (recedbytes == -1)
        {
            printf("Receive error");
            close(sockfd);
            close(newsockfd);
            return 1;
        }

        /* Check if client wants to stop the conversation */
        if (buff[0] == 's' && buff[1] == 't' && buff[2] == 'o' && buff[3] == 'p')
        {
            break;
        }

        /* Print the received message */
        printf("%s\n", buff);

        /* Clear buffer before sending response */
        memset(buff, '\0', sizeof(buff));

        /* Read response from server input (no bounds checking - unsafe) */
        scanf("%s", buff);

        /* Ensure string is null-terminated */
        buff[strlen(buff)] = '\0';

        /* Send response to the client through the socket
         * newsockfd: Socket file descriptor for the client connection
         * buff: Buffer containing data to send
         * strlen(buff) * sizeof(char): Number of bytes to send
         * 0: No special flags
         * Returns: Number of bytes sent, -1 on error
         */
        sentbytes = send(newsockfd, buff, strlen(buff) * sizeof(char), 0);

        /* Check if send failed */
        if (sentbytes == -1)
        {
            printf("Send error");
            close(sockfd);
            close(newsockfd);
            return 1;
        }

        /* Check if server wants to stop the conversation */
        if (buff[0] == 's' && buff[1] == 't' && buff[2] == 'o' && buff[3] == 'p')
        {
            break;
        }
    }

    /* Close the sockets to release resources */
    close(newsockfd);
    close(sockfd);

    return 0;
}
