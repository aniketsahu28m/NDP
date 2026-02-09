/* Lab 4 Q2_Unbound: DNS Client (TCP)
 * Client for the recursive DNS server
 * Sends domain name queries and receives resolved IP addresses
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 5003
#define MAX 512

int main(void)
{
    int sockfd, retval;
    struct sockaddr_in servaddr;
    char domain[MAX], response[MAX];

    /* Create TCP socket
     * AF_INET: IPv4 address family
     * SOCK_STREAM: TCP socket for reliable communication
     */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("Socket creation failed\n");
        exit(0);
    }

    /* Configure server address */
    memset(&servaddr, '\0', sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    /* Connect to DNS server */
    retval = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if (retval == -1)
    {
        printf("Connection to DNS server failed\n");
        close(sockfd);
        exit(0);
    }

    printf("========================================\n");
    printf("Connected to Recursive DNS Server\n");
    printf("Server: 127.0.0.1:%d\n", PORT);
    printf("========================================\n");
    printf("Enter domain names to resolve.\n");
    printf("Type 'exit' to quit.\n\n");

    /* Main query loop */
    while (1)
    {
        printf("Enter domain name: ");
        scanf("%s", domain);

        /* Send domain query to server */
        send(sockfd, domain, strlen(domain), 0);

        /* Check for exit command */
        if (strcmp(domain, "exit") == 0)
        {
            printf("Disconnecting from server...\n");
            break;
        }

        /* Receive and display response */
        memset(response, 0, MAX);
        if (recv(sockfd, response, MAX, 0) <= 0)
        {
            printf("Server disconnected\n");
            break;
        }

        printf("DNS Response: %s\n\n", response);
    }

    printf("Connection closed.\n");
    close(sockfd);
    return 0;
}
