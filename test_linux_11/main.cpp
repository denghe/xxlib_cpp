#include <sys/epoll.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
int main(void) {
    char buffer[4096];
    int fd = epoll_create1(0);
    struct epoll_event event;
    bzero(&event, sizeof(event));
    event.events = EPOLLIN;
    event.data.fd = 0;
    epoll_ctl(fd, EPOLL_CTL_ADD, STDIN_FILENO, &event);
    for (;;) {
        epoll_wait(fd, &event, 1, -1);
        auto n = read(event.data.fd, buffer, sizeof(buffer) - 1);
        if (n <= 0) return -1;
        buffer[n] = '\0';
        for (int i = 0; i < n; i++) {
            printf("%d ", (int)buffer[i]);
        }
        printf("\n");
    }
}
