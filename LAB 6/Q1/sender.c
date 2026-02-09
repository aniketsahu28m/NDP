// sender.c - Transmit binary data with parity bit
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 5006
#define MAX 256

int main() {
    int sockfd;
    struct sockaddr_in servaddr;
    char data[MAX];
    int parity_type;

    // Input binary data
    printf("Enter binary data (e.g., 101101): ");
    scanf("%s", data);

    // Input parity type
    printf("Enter parity type (0=even, 1=odd): ");
    scanf("%d", &parity_type);

    // Calculate parity bit
    int ones = 0;
    for(int i=0; i<strlen(data); i++)
        if(data[i]=='1') ones++;

    char parity_bit;
    if(parity_type == 0) // even parity
        parity_bit = (ones % 2 == 0) ? '0' : '1';
    else // odd parity
        parity_bit = (ones % 2 == 0) ? '1' : '0';

    // Append parity bit
    int len = strlen(data);
    data[len] = parity_bit;
    data[len+1] = '\0';

    printf("Data with parity bit: %s\n", data);

    // Create TCP socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to receiver
    if(connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("Connect failed");
        exit(1);
    }

    // Send data
    send(sockfd, data, strlen(data), 0);
    printf("Data sent to receiver.\n");

    close(sockfd);
    return 0;
}
