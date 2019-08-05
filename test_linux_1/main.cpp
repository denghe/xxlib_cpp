#include "xx_epoll_context.h"

int main() {

	// echo server sample
	xx::EpollListen(1234, xx::SockTypes::TCP, 2, [](int fd, auto read, auto write) {
			printf("peer accepted. fd = %i\n", fd);
			char buf[1024];
			while (size_t received = read(buf, sizeof(buf))) {
				if (write(buf, received)) break;
			}
			printf("peer disconnected: fd = %i\n", fd);
		}
	);
}
