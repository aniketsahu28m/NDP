#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>

void compute_crc(char *data, char *gen, char *remainder) {
    int n = strlen(data);
    int g = strlen(gen);
    char temp[512];
    strcpy(temp, data);
    
    // Append g-1 zeros to data
    for (int i = 0; i < g - 1; i++) temp[n + i] = '0';
    temp[n + g - 1] = '\0';

    // Perform XOR division
    for (int i = 0; i < n; i++) {
        if (temp[i] == '1') {
            for (int j = 0; j < g; j++) {
                temp[i + j] = (temp[i + j] == gen[j]) ? '0' : '1';
            }
        }
    }
    // The remainder is the last g-1 bits
    strncpy(remainder, temp + n, g - 1);
    remainder[g - 1] = '\0';
}

int main() {
    int sockfd;
    struct sockaddr_in servaddr;
    char data[256], gen[64], remainder[64], final_packet[512];
    int choice;

    printf("Enter binary data: ");
    scanf("%s", data);
    printf("Select CRC Polynomial:\n");
    printf("1. CRC-12 (1100000001111)\n");
    printf("2. CRC-16 (11000000000000101)\n");
    printf("3. CRC-CCITT (10001000000100001)\n");
    printf("Choice: ");
    scanf("%d", &choice);

    if(choice == 1) strcpy(gen, "1100000001111");
    else if(choice == 2) strcpy(gen, "11000000000000101");
    else strcpy(gen, "10001000000100001");

    compute_crc(data, gen, remainder);
    sprintf(final_packet, "%s%s", data, remainder);
    
    printf("\n--- CRC Generation ---\n");
    printf("Generator: %s\n", gen);
    printf("Checksum:  %s\n", remainder);
    printf("Sending:   %s\n", final_packet);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(5007);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("Connection failed");
        return 1;
    }

    send(sockfd, final_packet, strlen(final_packet), 0);
    usleep(1000); // Small delay to separate packets
    send(sockfd, gen, strlen(gen), 0); 

    printf("Data transmitted successfully.\n");
    close(sockfd);
    return 0;
}