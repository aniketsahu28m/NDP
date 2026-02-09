// Lab 4 Q1: Student Database Client (TCP)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 5000
#define MAX 1024

int main() {
    int sockfd;
    struct sockaddr_in servaddr;
    char buf[MAX];
    int opt;

    sockfd = socket(AF_INET, SOCK_STREAM, 0); // Create TCP socket
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_port = htons(PORT); // Server port
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Localhost

    connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)); // Connect to server
    printf("Connected to server\n");

    while(1) {
        printf("\n1. Registration Number\n2. Name\n3. Subject Code\n4. Exit\nChoice: ");
        scanf("%d", &opt);
        getchar(); // Clear newline

        send(sockfd, &opt, sizeof(int), 0); // Send option to server

        if(opt == 4) break;

        printf("Enter query: ");
        fgets(buf, MAX, stdin);
        buf[strcspn(buf, "\n")] = 0; // Remove trailing newline

        send(sockfd, buf, strlen(buf), 0); // Send query to server

        memset(buf, 0, MAX);
        recv(sockfd, buf, MAX, 0); // Receive response from server
        printf("Server Response: %s\n", buf);
    }

    close(sockfd);
    return 0;
}
