// Lab 4 Q1: Student Database Client (UDP)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 5001
#define MAX 1024

int main() {
    int sockfd;
    struct sockaddr_in servaddr;
    socklen_t len = sizeof(servaddr);
    char buf[MAX];
    int opt;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0); // Create UDP socket
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    printf("UDP Client ready\n");

    while(1) {
        printf("\n1. Registration Number\n2. Name\n3. Subject Code\n4. Exit\nChoice: ");
        scanf("%d", &opt);
        getchar();

        sendto(sockfd, &opt, sizeof(int), 0, (struct sockaddr*)&servaddr, len); // Send option

        if(opt == 4) break;

        printf("Enter query: ");
        fgets(buf, MAX, stdin);
        buf[strcspn(buf, "\n")] = 0;

        sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr*)&servaddr, len); // Send query

        memset(buf, 0, MAX);
        recvfrom(sockfd, buf, MAX, 0, (struct sockaddr*)&servaddr, &len); // Receive response
        printf("Server Response: %s\n", buf);
    }

    close(sockfd);
    return 0;
}
