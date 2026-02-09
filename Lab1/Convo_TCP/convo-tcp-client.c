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
    struct sockaddr_in serveraddr;
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
        printf("\nSocket Creation Error");
        return 1;
    }

    /* Configure server address structure
     * sin_family: Address family (IPv4)
     * sin_port: Port number in network byte order (htons converts host to network short)
     * sin_addr.s_addr: IP address (127.0.0.1 = localhost, inet_addr converts string to network order)
     */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(3388);
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    /* Initiate connection to the server
     * sockfd: Socket file descriptor
     * (struct sockaddr*)&serveraddr: Pointer to server address structure
     * sizeof(serveraddr): Size of address structure
     * Returns: 0 on success, -1 on error
     */
    retval = connect(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));

    /* Check if connection failed */
    if (retval == -1)
    {
        printf("Connection error");
        return 1;
    }

    /* Conversation loop - continuously send and receive messages */
    while (1)
    {
        /* Clear buffer to receive new input */
        memset(buff, '\0', sizeof(buff));

        /* Prompt user for input */
        printf("enter the text\n");

        /* Read string from user input (no bounds checking - unsafe) */
        scanf("%s", buff);

        /* Ensure string is null-terminated */
        buff[strlen(buff)] = '\0';

        /* Send data to the server through the socket
         * sockfd: Socket file descriptor
         * buff: Buffer containing data to send
         * strlen(buff) * sizeof(char): Number of bytes to send
         * 0: No special flags
         * Returns: Number of bytes sent, -1 on error
         */
        sentbytes = send(sockfd, buff, strlen(buff) * sizeof(char), 0);

        /* Check if send failed */
        if (sentbytes == -1)
        {
            printf("Send error");
            close(sockfd);
            return 1;
        }

        /* Check if user wants to stop the conversation */
        if (buff[0] == 's' && buff[1] == 't' && buff[2] == 'o' && buff[3] == 'p')
        {
            break;
        }

        /* Clear buffer before receiving response */
        memset(buff, '\0', sizeof(buff));

        /* Receive response from the server
         * sockfd: Socket file descriptor
         * buff: Buffer to store received data
         * sizeof(buff): Maximum number of bytes to receive
         * 0: No special flags
         * Returns: Number of bytes received (0 if connection closed), -1 on error
         */
        recedbytes = recv(sockfd, buff, sizeof(buff), 0);

        /* Print the received message */
        printf("%s\n", buff);

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
