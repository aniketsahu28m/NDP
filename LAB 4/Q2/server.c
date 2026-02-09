// Lab 4 Q2: DNS Server (TCP) - Maps domain names to IP addresses
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 5002
#define MAX 256

// DNS database: domain name -> IP address mapping
struct DNS { char domain[50]; char ip[20]; };
struct DNS db[] = {
    {"www.google.com", "142.250.195.36"},
    {"www.facebook.com", "157.240.1.35"},
    {"www.amazon.com", "205.251.242.103"},
    {"www.mit.edu", "104.18.29.39"},
    {"www.manipal.edu", "103.26.236.3"}
};
int db_size = 5;

int main() {
    int sockfd, newsockfd;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len = sizeof(cliaddr);
    char domain[MAX], response[MAX];

    sockfd = socket(AF_INET, SOCK_STREAM, 0); // Create TCP socket
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    listen(sockfd, 5);
    printf("DNS Server running on port %d\n", PORT);

    newsockfd = accept(sockfd, (struct sockaddr*)&cliaddr, &len);

    while(1) {
        memset(domain, 0, MAX);
        memset(response, 0, MAX);

        if(recv(newsockfd, domain, MAX, 0) <= 0) break; // Receive domain name query

        if(strcmp(domain, "exit") == 0) break;

        strcpy(response, "Domain not found"); // Default response

        for(int i = 0; i < db_size; i++) { // Search in DNS database
            if(strcmp(db[i].domain, domain) == 0) {
                sprintf(response, "IP Address: %s", db[i].ip);
                break;
            }
        }

        send(newsockfd, response, strlen(response), 0); // Send IP address to client
    }

    close(newsockfd); close(sockfd);
    return 0;
}
