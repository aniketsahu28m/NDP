// Lab 4 Q1: Student Database Server (UDP) - Uses fork() for 3 child processes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>

#define PORT 5001
#define MAX 1024

struct Student { char reg[20], name[50], addr[100], dept[20], sem[10], sec[5], courses[100]; int marks; };
struct Student db[] = {
    {"22MIT1001", "Alice", "Hostel A, Manipal", "IT", "5", "A", "NPS,AI,ML", 85},
    {"22MIT1002", "Bob", "Hostel B, Manipal", "IT", "5", "B", "NPS,OS,DBMS", 78},
    {"22MIT1003", "Charlie", "Hostel C, Manipal", "CSE", "5", "A", "NPS,CN,SE", 92}
};
int db_size = 3;

int main() {
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len = sizeof(cliaddr);
    char buf[MAX];
    int opt;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0); // Create UDP socket
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    printf("UDP Server listening on port %d\n", PORT);

    while(1) {
        memset(buf, 0, MAX);
        recvfrom(sockfd, &opt, sizeof(int), 0, (struct sockaddr*)&cliaddr, &len); // Receive option
        recvfrom(sockfd, buf, MAX, 0, (struct sockaddr*)&cliaddr, &len); // Receive query

        if(opt == 4) break;

        pid_t pid = fork();
        if(pid == 0) {
            char response[MAX] = "Not found";

            for(int i = 0; i < db_size; i++) {
                if(opt == 1 && strcmp(db[i].reg, buf) == 0) {
                    sprintf(response, "Name: %s, Address: %s (Child PID: %d)", db[i].name, db[i].addr, getpid());
                    break;
                } else if(opt == 2 && strcmp(db[i].name, buf) == 0) {
                    sprintf(response, "Dept: %s, Sem: %s, Sec: %s, Courses: %s (Child PID: %d)",
                            db[i].dept, db[i].sem, db[i].sec, db[i].courses, getpid());
                    break;
                } else if(opt == 3 && strstr(db[i].courses, buf)) {
                    sprintf(response, "Student: %s, Marks: %d (Child PID: %d)", db[i].name, db[i].marks, getpid());
                    break;
                }
            }
            sendto(sockfd, response, strlen(response), 0, (struct sockaddr*)&cliaddr, len); // Send response
            exit(0);
        }
        wait(NULL);
    }

    close(sockfd);
    return 0;
}
