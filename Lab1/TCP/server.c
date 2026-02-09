#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#define MAXSIZE 90

int main(void)
{
    int sockfd, newsockfd, retval;
    socklen_t actuallen;
    int recedbytes, sentbytes;
    struct sockaddr_in serveraddr, clientaddr;
    char buff[MAXSIZE];
    int a = 0;

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
    }

    /* Configure server address structure
     * sin_family: Address family (IPv4)
     * sin_port: Port number in network byte order (htons converts host to network short)
     * sin_addr.s_addr: IP address (INADDR_ANY = any local IP, htons not needed here)
     */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(3388);
    serveraddr.sin_addr.s_addr = htons(INADDR_ANY);

    /* Bind the socket to the configured address and port
     * sockfd: Socket file descriptor
     * (struct sockaddr*)&serveraddr: Pointer to address structure (cast for compatibility)
     * sizeof(serveraddr): Size of address structure
     * Returns: 0 on success, -1 on error
     */
    retval = bind(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));

    /* Check if binding failed */
    if (retval == -1)
    {
        printf("Binding error");
        close(sockfd);
    }

    /* Mark the socket as passive (listening for incoming connections)
     * sockfd: Socket file descriptor
     * 1: Backlog queue size (maximum pending connections)
     * Returns: 0 on success, -1 on error
     */
    retval = listen(sockfd, 1);

    /* Check if listen failed */
    if (retval == -1)
    {
        close(sockfd);
    }

    /* Initialize actuallen with the size of clientaddr structure
     * This is required for accept() to know the buffer size
     */
    actuallen = sizeof(clientaddr);

    /* Accept an incoming connection from a client
     * Blocks until a client connects
     * Returns: New socket file descriptor for the connection on success, -1 on error
     */
    newsockfd = accept(sockfd, (struct sockaddr*)&clientaddr, &actuallen);

    /* Check if accept failed */
    if (newsockfd == -1)
    {
        close(sockfd);
    }

    /* During debugging, go to client HERE. */

    /* Receive data from the client through the connected socket
     * newsockfd: Socket file descriptor for the connection
     * buff: Buffer to store received data
     * sizeof(buff): Maximum number of bytes to receive
     * 0: No special flags
     * Returns: Number of bytes received (0 if connection closed), -1 on error
     */
    recedbytes = recv(newsockfd, buff, sizeof(buff), 0);

    /* Check if recv failed */
    if (recedbytes == -1)
    {
        close(sockfd);
        close(newsockfd);
    }

    /* Print the received message */
    puts(buff);
    printf("\n");

    /* Read input from user (keyboard) */
    scanf("%s", buff);

    /* Send data back to the client through the connected socket
     * newsockfd: Socket file descriptor for the connection
     * buff: Buffer containing data to send
     * sizeof(buff): Number of bytes to send
     * 0: No special flags
     * Returns: Number of bytes sent, -1 on error
     */
    sentbytes = send(newsockfd, buff, sizeof(buff), 0);

    /* Check if send failed */
    if (sentbytes == -1)
    {
        close(sockfd);
        close(newsockfd);
    }

    /* Close the sockets to release resources
     * Order matters: close newsockfd first, then sockfd
     */
    close(sockfd);
    close(newsockfd);

    return 0;
}
