#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 5007

int main() {
    int sockfd;
    struct sockaddr_in servaddr;
    int d[8];
    char packet[8];

    printf("--- Hamming Sender (4-bit to 7-bit) ---\n");
    printf("Enter 4 data bits (e.g., 1 0 1 1):\n");
    scanf("%d %d %d %d", &d[3], &d[5], &d[6], &d[7]);

    // Hamming Encoding Logic
    d[1] = d[3] ^ d[5] ^ d[7]; // Parity 1
    d[2] = d[3] ^ d[6] ^ d[7]; // Parity 2
    d[4] = d[5] ^ d[6] ^ d[7]; // Parity 4

    for(int i = 1; i <= 7; i++) packet[i-1] = d[i] + '0';
    packet[7] = '\0';

    printf("Encoded Hamming Code to send: %s\n", packet);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if(connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("Connect failed. Is the receiver running?");
        return 1;
    }

    send(sockfd, packet, 7, 0);
    printf("Data sent successfully to Port %d.\n", PORT);

    close(sockfd);
    return 0;
}