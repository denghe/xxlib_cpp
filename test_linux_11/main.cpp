#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <thread>
#include <vector>

#define PORT     8080 
#define MAXLINE  65536 

// Driver code 
int main() {
	std::vector<std::thread> ts;
	for (int i = 0; i < 1; ++i) {
		ts.emplace_back([] {
			int sockfd;
			char buffer[MAXLINE];
			struct sockaddr_in servaddr;

			// Creating socket file descriptor 
			if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
				perror("socket creation failed");
				exit(EXIT_FAILURE);
			}

			memset(&servaddr, 0, sizeof(servaddr));

			// Filling server information 
			servaddr.sin_family = AF_INET; // IPv4 
			servaddr.sin_addr.s_addr = INADDR_ANY;
			servaddr.sin_port = htons(PORT);
			if (!inet_pton(AF_INET, "192.168.1.236", &servaddr.sin_addr)) return -1;

			socklen_t caLen = sizeof(sockaddr_in6);
			while (true) {
				sendto(sockfd, ".", 1, MSG_CONFIRM, (const struct sockaddr*) & servaddr, caLen);
				//auto n = recvfrom(sockfd, (char*)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr*) & servaddr, &caLen);
			}
			});
	}
	for (auto&& t : ts) {
		t.join();
	}
	return 0;
}
