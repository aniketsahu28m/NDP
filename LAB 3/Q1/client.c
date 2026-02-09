#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#define MAXSIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in serveraddr;
    char buff[MAXSIZE];
    pid_t cpid;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(8080);
    // REPLACE THIS IP WITH THE SERVER'S IP ADDRESS
    serveraddr.sin_addr.s_addr = inet_addr("172.18.175.71"); 

    if (connect(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) == -1) {
        printf("Connection failed.\n");
        exit(1);
    }
    printf("Connected to server. Type 'exit' to quit.\n");

    cpid = fork();

    if (cpid == 0) { // Child: Receiving
        while (1) {
            memset(buff, 0, MAXSIZE);
            if (recv(sockfd, buff, MAXSIZE, 0) <= 0 || strcmp(buff, "exit") == 0) {
                printf("\nServer disconnected. Press Enter to exit.\n");
                kill(getppid(), SIGTERM);
                exit(0);
            }
            printf("\nServer: %s\nClient: ", buff);
            fflush(stdout);
        }
    } else { // Parent: Sending
        while (1) {
            printf("Client: ");
            fgets(buff, MAXSIZE, stdin);
            buff[strcspn(buff, "\n")] = 0;
            send(sockfd, buff, strlen(buff) + 1, 0);
            if (strcmp(buff, "exit") == 0) break;
        }
        kill(cpid, SIGTERM);
    }

    close(sockfd);
    return 0;
}