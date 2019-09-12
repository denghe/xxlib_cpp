#include "xx_epoll_context.h"
#include <signal.h>

int main() {
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;//设定接受到指定信号后的动作为忽略
	sa.sa_flags = 0;
	if (sigemptyset(&sa.sa_mask) == -1 || //初始化信号集为空
		sigaction(SIGPIPE, &sa, 0) == -1) { //屏蔽SIGPIPE信号
		perror("failed to ignore SIGPIPE; sigaction");
		exit(EXIT_FAILURE);
	}

	sigset_t signal_mask;
	sigemptyset(&signal_mask);
	sigaddset(&signal_mask, SIGPIPE);
	int rc = pthread_sigmask(SIG_BLOCK, &signal_mask, NULL);
	if (rc != 0) {
		printf("block sigpipe error\n");
	}

	// echo server sample
	xx::EpollListen(12344, xx::SockTypes::TCP, 3, [](int fd, auto read, auto write) {
			printf("peer accepted. fd = %i\n", fd);
			char buf[1024];
			while (size_t received = read(buf, sizeof(buf))) {
				if (write(buf, received)) break;
			}
			printf("peer disconnected: fd = %i\n", fd);
		}
	);
	return 0;
}
