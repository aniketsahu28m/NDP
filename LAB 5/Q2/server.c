// Lab 5 Q2: Iterative Server - Anagram check with date/time display
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <ctype.h>

#define PORT 5005
#define MAX 256

// Check if two strings are anagrams
int isAnagram(char *s1, char *s2) {
    int count[256] = {0};
    if(strlen(s1) != strlen(s2)) return 0;
    for(int i = 0; s1[i]; i++) { count[(int)tolower(s1[i])]++; count[(int)tolower(s2[i])]--; }
    for(int i = 0; i < 256; i++) if(count[i] != 0) return 0;
    return 1;
}

int main() {
    int sockfd, newsockfd;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len;
    char str1[MAX], str2[MAX], response[MAX];
    time_t t; struct tm *tm_info; char timestr[64];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    listen(sockfd, 5);
    printf("Iterative Server on port %d\n", PORT);

    while(1) { // Iterative: handle one client at a time
        len = sizeof(cliaddr);
        newsockfd = accept(sockfd, (struct sockaddr*)&cliaddr, &len);

        // Get current date/time
        t = time(NULL); tm_info = localtime(&t);
        strftime(timestr, 64, "%Y-%m-%d %H:%M:%S", tm_info);
        printf("Client %s:%d connected at %s\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port), timestr);

        // Receive two strings
        memset(str1, 0, MAX); memset(str2, 0, MAX);
        recv(newsockfd, str1, MAX, 0);
        recv(newsockfd, str2, MAX, 0);

        // Check anagram and prepare response
        if(isAnagram(str1, str2))
            sprintf(response, "'%s' and '%s' are ANAGRAMS", str1, str2);
        else
            sprintf(response, "'%s' and '%s' are NOT anagrams", str1, str2);

        send(newsockfd, response, strlen(response), 0);
        close(newsockfd);
    }

    close(sockfd);
    return 0;
}
