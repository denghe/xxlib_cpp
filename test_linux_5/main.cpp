#include "xx_simplehttpserver.h"

struct MyHttpServer : xx::SimpleHttpServer {
	MyHttpServer() {

		handlers[""] = [this]()->int {
			return SendResponse_HtmlBody(R"--(
<a href="/cmd1">cmd1</a></br>
<a href="/cmd2">cmd2</a></br>
<a href="/cmd3">cmd3</a>
)--");
		};

		handlers["cmd1"] = [this]()->int {
			return SendResponse_HtmlBody(R"--(
todo1</br>
<a href="/">home</a>
)--");
		};

		handlers["cmd2"] = [this]()->int {
			return SendResponse_HtmlBody(R"--(
todo2</br>
<a href="/">home</a>
)--");
		};

		handlers["cmd3"] = [this]()->int {
			return SendResponse_HtmlBody(R"--(
todo3</br>
<a href="/">home</a>
)--");
		};
	}
};

int main() {
	xx::IgnoreSignal();
	auto&& server = std::make_unique<MyHttpServer>();
	int r = server->Listen(12345);
	assert(!r);
	server->Run(1);
	return 0;
}
