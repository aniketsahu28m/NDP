// Lab 5 Q1: Client 1 - Sends "Institute Of" to server
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 5004
#define MAX 256

int main() {
    int sockfd;
    struct sockaddr_in servaddr;
    char buf[MAX] = "Institute Of"; // Fixed string for client1

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    printf("Client1 connected. Sending: %s\n", buf);

    send(sockfd, buf, strlen(buf), 0); // Send "Institute Of"

    memset(buf, 0, MAX);
    recv(sockfd, buf, MAX, 0); // Receive result
    printf("Server Response:\n%s\n", buf);

    close(sockfd);
    return 0;
}
