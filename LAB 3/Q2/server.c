#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

#define MAXSIZE 1024

/* Function to swap two characters */
void swap(char *x, char *y)
{
    char temp;
    temp = *x;
    *x = *y;
    *y = temp;
}

/* Function to generate all permutations of string
 * Uses backtracking to generate permutations recursively
 */
void permute(char *a, int l, int r)
{
    int i;

    if (l == r)
    {
        printf("%s\n", a);
    }
    else
    {
        for (i = l; i <= r; i++)
        {
            swap((a + l), (a + i));
            permute(a, l + 1, r);
            swap((a + l), (a + i));
        }
    }
}

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
        printf("\nSocket creation error");
        exit(0);
    }

    /* Configure server address structure
     * sin_family: Address family (IPv4)
     * sin_port: Port number in network byte order
     * sin_addr.s_addr: Local IP address
     */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(6767);
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    /* Bind the socket to the configured address and port */
    retval = bind(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));

    if (retval == -1)
    {
        printf("Binding error");
        close(sockfd);
        exit(0);
    }

    actuallen = sizeof(clientaddr);

    /* Receive string from client */
    recedbytes = recvfrom(sockfd, buff, sizeof(buff), 0, (struct sockaddr*)&clientaddr, &actuallen);

    if (recedbytes == -1)
    {
        close(sockfd);
        exit(0);
    }

    /* Get process information */
    printf("Server PID: %d, PPID: %d\n", getpid(), getppid());

    /* Print received string */
    printf("Received string from client: %s\n", buff);

    /* Generate and print all permutations */
    printf("All permutations of the string:\n");
    permute(buff, 0, strlen(buff) - 1);

    /* Send completion message to client */
    strcpy(buff, "Permutations calculated");
    sentbytes = sendto(sockfd, buff, sizeof(buff), 0, (struct sockaddr*)&clientaddr, actuallen);

    if (sentbytes == -1)
    {
        close(sockfd);
        exit(0);
    }

    close(sockfd);

    return 0;
}
