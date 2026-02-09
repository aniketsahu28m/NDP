// Lab 4 Q2: DNS Client (TCP)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 5002
#define MAX 256

int main() {
    int sockfd;
    struct sockaddr_in servaddr;
    char domain[MAX], response[MAX];

    sockfd = socket(AF_INET, SOCK_STREAM, 0); // Create TCP socket
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    printf("Connected to DNS Server\n");

    while(1) {
        printf("\nEnter domain name (or 'exit'): ");
        scanf("%s", domain);

        send(sockfd, domain, strlen(domain), 0); // Send domain query

        if(strcmp(domain, "exit") == 0) break;

        memset(response, 0, MAX);
        recv(sockfd, response, MAX, 0); // Receive IP address
        printf("DNS Response: %s\n", response);
    }

    close(sockfd);
    return 0;
}
