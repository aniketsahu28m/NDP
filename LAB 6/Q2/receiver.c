#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>

void verify_crc(char *packet, char *gen) {
    int p_len = strlen(packet);
    int g_len = strlen(gen);
    char temp[512];
    strcpy(temp, packet);

    // Perform XOR division on the received packet
    for (int i = 0; i <= p_len - g_len; i++) {
        if (temp[i] == '1') {
            for (int j = 0; j < g_len; j++) {
                temp[i + j] = (temp[i + j] == gen[j]) ? '0' : '1';
            }
        }
    }

    // If any bit in the remainder is '1', there is an error
    int error = 0;
    for (int i = 0; i < p_len; i++) {
        if (temp[i] == '1') {
            error = 1;
            break;
        }
    }

    printf("\n--- Receiver Result ---\n");
    if (error) 
        printf("Result: Remainder is non-zero. Data is CORRUPTED!\n");
    else 
        printf("Result: Remainder is zero. Data is NOT corrupted.\n");
}

int main() {
    int sockfd, newsock;
    struct sockaddr_in addr;
    char packet[512], gen[64];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(5007);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Bind failed");
        return 1;
    }
    
    listen(sockfd, 5);
    printf("Receiver listening on port 5007...\n");

    newsock = accept(sockfd, NULL, NULL);
    
    memset(packet, 0, 512);
    memset(gen, 0, 64);
    
    recv(newsock, packet, 512, 0);
    recv(newsock, gen, 64, 0);

    printf("Received Packet: %s\n", packet);
    printf("Using Polynomial: %s\n", gen);
    
    verify_crc(packet, gen);

    close(newsock);
    close(sockfd);
    return 0;
}