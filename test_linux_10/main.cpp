#include "xx_epoll2_http.h"
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

	ep.cmds["exit"] = [&](auto args) { ep.running = false; };
	ep.cmds["quit"] = ep.cmds["exit"];
}


int main() {
	xx::IgnoreSignal();
	EP::Context ep;
	auto&& listener = ep.CreateTcpListener<EP::HttpListener>(12312);
	if (!listener) return -1;
	Bind(ep, listener);
	ep.Run(10);
	return 0;
}
