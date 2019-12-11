#include "xx_epoll2_http.h"
namespace EP = xx::Epoll;

void Bind(EP::Ref<EP::HttpListener> const& listener) {
    listener->handlers[""] = [](xx::HttpContext& q, xx::HttpResponse& r) {
        char const str[] = R"--(
<html><body>
<p>hi</p>
</body></html>
)--";
        r.onSend(r.prefixHtml, str, sizeof(str));
    };
}

int main() {
    EP::Context ep;
    auto&& listener = ep.CreateTcpListener<EP::HttpListener>(12312);
    if (!listener) return -1;
    Bind(listener);
    std::shared_ptr<int> s;

	std::vector<std::thread> threads;
	for (int i = 2; i <= 6; ++i) {
		threads.emplace_back([fd = listener->fd]{

			EP::Context ep;
			auto listener = ep.CreateSharedTcpListener<EP::HttpListener>(fd);
			if (!listener) throw - 1;
            Bind(listener);
			ep.Run(1);

			}).detach();
	}

    ep.Run(1);
    return 0;
}
