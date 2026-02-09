// Lab 5 Q2: Client - Sends two strings for anagram check
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 5005
#define MAX 256

int main() {
    int sockfd;
    struct sockaddr_in servaddr;
    char str1[MAX], str2[MAX], response[MAX];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    printf("Connected to server\n");

    printf("Enter first string: "); scanf("%s", str1);
    printf("Enter second string: "); scanf("%s", str2);

    send(sockfd, str1, strlen(str1), 0); // Send first string
    send(sockfd, str2, strlen(str2), 0); // Send second string

    memset(response, 0, MAX);
    recv(sockfd, response, MAX, 0); // Receive result
    printf("Server: %s\n", response);

    close(sockfd);
    return 0;
}
