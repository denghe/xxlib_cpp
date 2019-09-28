#include "xx_epoll.h"

int main()
{

	int pipeFDs[2];
	auto&& r = pipe(pipeFDs);
	assert(r >= 0);

	std::thread t([&] {
		while (true) {
			auto&& ticks = xx::NowEpoch10m();
			auto&& r = write(pipeFDs[1], &ticks, sizeof(ticks));
			assert(r == sizeof(ticks));
			usleep(10000);
		}
	});

	auto&& efd = epoll_create1(0);
	if (-1 == efd) throw - 1;

	epoll_event event;
	event.data.fd = pipeFDs[0];
	event.events = EPOLLIN;
	r = epoll_ctl(efd, EPOLL_CTL_ADD, pipeFDs[0], &event);
	assert(r == 0);

	std::array<epoll_event, 512> events;
	while (true) {
		int n = epoll_wait(efd, events.data(), events.size(), -1);
		if (n == -1) return errno;
		for (int i = 0; i < n; ++i) {
			auto fd = events[i].data.fd;
			auto ev = events[i].events;

			// error
			if (ev & EPOLLERR || ev & EPOLLHUP) {
				epoll_ctl(efd, EPOLL_CTL_DEL, fd, nullptr);
				continue;
			}

			// read
			if (ev & EPOLLIN) {
				if (fd == pipeFDs[0]) {
					int64_t ticks;
					auto&& len = read(fd, &ticks, sizeof(ticks));
					if (len <= 0) return -1;
					xx::CoutN(xx::NowEpoch10m() - ticks);
				}
			}
		}
	}

	//sleep(3);
	//write(fd[1], ...)
	//read(fd[0], readbuf, sizeof(readbuf));


	return 0;
}
