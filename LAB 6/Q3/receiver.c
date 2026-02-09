#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 5007

int main() {
    int sockfd, newsock;
    struct sockaddr_in addr;
    char buffer[8];
    int r[8];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    // Set socket option to reuse port (prevents "Address already in use" errors)
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Bind failed");
        return 1;
    }

    listen(sockfd, 5);
    printf("Receiver WAITING on port %d for Hamming data...\n", PORT);

    newsock = accept(sockfd, NULL, NULL);
    memset(buffer, 0, 8);
    recv(newsock, buffer, 7, 0);
    
    printf("Received Codeword: %s\n", buffer);
    for(int i = 1; i <= 7; i++) r[i] = buffer[i-1] - '0';

    // Syndrome Calculation
    int s1 = r[1] ^ r[3] ^ r[5] ^ r[7];
    int s2 = r[2] ^ r[3] ^ r[6] ^ r[7];
    int s4 = r[4] ^ r[5] ^ r[6] ^ r[7];

    int syndrome = (s4 * 4) + (s2 * 2) + s1;

    printf("\n--- Hamming Analysis ---\n");
    if(syndrome == 0) {
        printf("Result: No error detected.\n");
    } else {
        printf("Result: Error detected at bit position: %d\n", syndrome);
        // Correct the flipped bit
        r[syndrome] = !r[syndrome]; 
        printf("Corrected Codeword: ");
        for(int i = 1; i <= 7; i++) printf("%d", r[i]);
        printf("\n");
    }

    printf("Original 4-bit Data: %d%d%d%d\n", r[3], r[5], r[6], r[7]);

    close(newsock);
    close(sockfd);
    return 0;
}