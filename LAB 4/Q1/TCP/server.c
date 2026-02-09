// Lab 4 Q1: Student Database Server (TCP) - Uses fork() for 3 child processes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>

#define PORT 5000
#define MAX 1024

// Student database structure
struct Student { char reg[20], name[50], addr[100], dept[20], sem[10], sec[5], courses[100]; int marks; };
struct Student db[] = {
    {"22MIT1001", "Alice", "Hostel A, Manipal", "IT", "5", "A", "NPS,AI,ML", 85},
    {"22MIT1002", "Bob", "Hostel B, Manipal", "IT", "5", "B", "NPS,OS,DBMS", 78},
    {"22MIT1003", "Charlie", "Hostel C, Manipal", "CSE", "5", "A", "NPS,CN,SE", 92}
};
int db_size = 3;

int main() {
    int sockfd, newsockfd; // Server and client socket descriptors
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len = sizeof(cliaddr);
    char buf[MAX];
    int opt;

    sockfd = socket(AF_INET, SOCK_STREAM, 0); // Create TCP socket
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY; // Accept from any interface
    servaddr.sin_port = htons(PORT); // Convert port to network byte order

    bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)); // Bind socket to address
    listen(sockfd, 5); // Listen for connections (max 5 in queue)
    printf("Server listening on port %d\n", PORT);

    newsockfd = accept(sockfd, (struct sockaddr*)&cliaddr, &len); // Accept client connection

    while(1) {
        memset(buf, 0, MAX);
        recv(newsockfd, &opt, sizeof(int), 0); // Receive option from client
        recv(newsockfd, buf, MAX, 0); // Receive query data

        if(opt == 4) break; // Exit option

        pid_t pid = fork(); // Create child process for handling request
        if(pid == 0) { // Child process
            char response[MAX] = "Not found";

            for(int i = 0; i < db_size; i++) {
                if(opt == 1 && strcmp(db[i].reg, buf) == 0) { // Option 1: Search by Reg No
                    sprintf(response, "Name: %s, Address: %s (Child PID: %d)", db[i].name, db[i].addr, getpid());
                    break;
                } else if(opt == 2 && strcmp(db[i].name, buf) == 0) { // Option 2: Search by Name
                    sprintf(response, "Dept: %s, Sem: %s, Sec: %s, Courses: %s (Child PID: %d)",
                            db[i].dept, db[i].sem, db[i].sec, db[i].courses, getpid());
                    break;
                } else if(opt == 3) { // Option 3: Search by Subject Code
                    if(strstr(db[i].courses, buf)) {
                        sprintf(response, "Student: %s, Marks: %d (Child PID: %d)", db[i].name, db[i].marks, getpid());
                        break;
                    }
                }
            }
            send(newsockfd, response, strlen(response), 0); // Send response to client
            exit(0); // Child exits after sending response
        }
        wait(NULL); // Parent waits for child to complete
    }

    close(newsockfd); close(sockfd);
    return 0;
}
