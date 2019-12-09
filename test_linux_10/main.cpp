#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include "xx_object.h"

#define PORT     8080 
#define MAXLINE  65536 

// Driver code 
int main() {
    int sockfd;
    char buffer[MAXLINE];
    struct sockaddr_in servaddr, cliaddr;

    // Creating socket file descriptor 
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Filling server information 
    servaddr.sin_family = AF_INET; // IPv4 
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // Bind the socket with the server address 
    if (bind(sockfd, (const struct sockaddr*) & servaddr,
        sizeof(servaddr)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    socklen_t caLen = sizeof(sockaddr_in6);
    size_t count = 0;
    auto ms = xx::NowSteadyEpochMS();
    while (true) {
        auto n = recvfrom(sockfd, (char*)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr*) & cliaddr, &caLen);
        //sendto(sockfd, buffer, n, MSG_CONFIRM, (const struct sockaddr*) & cliaddr, caLen);
        if (++count % 100000 == 0) {
            auto newMS = xx::NowSteadyEpochMS();
            xx::CoutN(newMS - ms);
            ms = newMS;
        }
    }

    return 0;
}
