#include "xx_epoll2_http.h"
#include <termios.h>
namespace EP = xx::Epoll;

void Bind(EP::Context& ep, EP::Ref<EP::HttpListener> const& listener) {
    listener->handlers[""] = [&](xx::HttpContext& q, xx::HttpResponse& r) {
        char const str[] = R"--(
<html><body>
<a href="exit">exit</p>
</body></html>
)--";
        r.onSend(r.prefixHtml, str, sizeof(str));
    };
    listener->handlers["exit"] = [&](xx::HttpContext& q, xx::HttpResponse& r) {
        ep.running = false;
    };
}

int main() {
    xx::IgnoreSignal();

    struct termios info;
    tcgetattr(0, &info);
    info.c_lflag &= ~(ICANON | ECHO);
    info.c_cc[VMIN] = 1;
    info.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &info);

    EP::Context ep;
    auto&& listener = ep.CreateTcpListener<EP::HttpListener>(12312);
    if (!listener) return -1;
    Bind(ep, listener);
    ep.CreateCommandHandler();
    ep.cmdHandlers["exit"] = [&](auto args) { ep.running = false; };
    ep.cmdHandlers["quit"] = ep.cmdHandlers["exit"];

	//std::vector<std::thread> threads;
	//for (int i = 2; i <= 6; ++i) {
	//	threads.emplace_back([fd = listener->fd]{

	//		EP::Context ep;
	//		auto listener = ep.CreateSharedTcpListener<EP::HttpListener>(fd);
	//		if (!listener) throw - 1;
 //           Bind(ep, listener);
	//		ep.Run(1);

	//		}).detach();
	//}

    ep.Run(1);
    return 0;
}
