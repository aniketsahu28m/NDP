// receiver.c - Receive binary data and check parity
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 5006
#define MAX 256

int main() {
    int sockfd, newsockfd;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len = sizeof(cliaddr);
    char data[MAX];

    // Create TCP socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    // Bind
    if(bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("Bind failed");
        exit(1);
    }

    listen(sockfd, 5);
    printf("Receiver waiting on port %d...\n", PORT);

    // Accept connection
    newsockfd = accept(sockfd, (struct sockaddr*)&cliaddr, &len);
    if(newsockfd < 0) {
        perror("Accept failed");
        exit(1);
    }

    // Receive data
    memset(data, 0, MAX);
    recv(newsockfd, data, MAX, 0);
    printf("Received data: %s\n", data);

    // Count number of 1s
    int ones = 0;
    for(int i=0; i<strlen(data); i++)
        if(data[i]=='1') ones++;

    // Check parity (assume even parity used)
    if(ones % 2 == 0)
        printf("Parity check: Data is NOT corrupt.\n");
    else
        printf("Parity check: Data is CORRUPT!\n");

    close(newsockfd);
    close(sockfd);
    return 0;
}
