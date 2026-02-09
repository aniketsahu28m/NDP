// Lab 5 Q1: Concurrent Server - 2 clients send strings, server appends to "Manipal"
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
    int sockfd, cli_fd[2];
    struct sockaddr_in servaddr, cliaddr[2];
    socklen_t len = sizeof(cliaddr[0]);
    char buf[MAX], result[MAX] = "Manipal"; // File keyword
    int client_count = 0;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    listen(sockfd, 5);
    printf("Concurrent Server on port %d. Waiting for 2 clients...\n", PORT);

    while(client_count < 2) { // Accept exactly 2 clients
        cli_fd[client_count] = accept(sockfd, (struct sockaddr*)&cliaddr[client_count], &len);
        printf("Client %d connected from %s:%d\n", client_count+1,
               inet_ntoa(cliaddr[client_count].sin_addr), ntohs(cliaddr[client_count].sin_port));
        client_count++;

        if(client_count > 2) { // Terminate if more than 2 clients
            send(cli_fd[client_count-1], "terminate session", 18, 0);
            close(cli_fd[client_count-1]);
            break;
        }
    }

    // Receive strings from both clients
    for(int i = 0; i < 2; i++) {
        memset(buf, 0, MAX);
        recv(cli_fd[i], buf, MAX, 0); // Receive string from client
        strcat(result, " "); strcat(result, buf); // Append to result
    }

    // Prepare final result with socket addresses
    char final[MAX * 2];
    sprintf(final, "%s\nClient1: %s:%d\nClient2: %s:%d", result,
            inet_ntoa(cliaddr[0].sin_addr), ntohs(cliaddr[0].sin_port),
            inet_ntoa(cliaddr[1].sin_addr), ntohs(cliaddr[1].sin_port));

    printf("Result: %s\n", final);

    // Send result to both clients
    for(int i = 0; i < 2; i++) {
        send(cli_fd[i], final, strlen(final), 0);
        close(cli_fd[i]);
    }

    close(sockfd);
    return 0;
}
