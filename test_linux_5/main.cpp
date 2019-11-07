#include "xx_simplehttpserver.h"


struct MyHtmlHandler {
	// 网址 path : 处理函数 映射填充到此
	std::unordered_map<std::string, std::function<int(xx::HttpContext & request, xx::HttpResponse & response)>> handlers;

	// 调用入口
	inline int operator()(xx::HttpContext& request, xx::HttpResponse& response) {
		// 填充 request.path 等
		request.ParseUrl();

		// 用 path 查找处理函数
		auto&& iter = handlers.find(request.path);

		// 如果没找到：输出默认报错页面
		if (iter == handlers.end()) {
			response.Send404Body("the page not found!");
		}
		// 找到则执行
		else {
			// 如果执行出错，输出默认报错页面
			if (iter->second(request, response)) {
				response.Send404Body("bad request!");
			}
		}
		return 0;
	}

	// 绑定处理函数
	MyHtmlHandler() {
		handlers[""] = [](xx::HttpContext& request, xx::HttpResponse& response)->int {
			return response.SendHtmlBody(R"--(
<a href="/cmd1">cmd1</a></br>
<a href="/cmd2">cmd2</a></br>
<a href="/cmd3">cmd3</a>
)--");
		};

		handlers["cmd1"] = [](xx::HttpContext& request, xx::HttpResponse& response)->int {
			return response.SendHtmlBody(R"--(
todo1</br>
<a href="/">home</a>
)--");
		};

		handlers["cmd2"] = [](xx::HttpContext& request, xx::HttpResponse& response)->int {
			return response.SendHtmlBody(R"--(
todo2</br>
<a href="/">home</a>
)--");
		};

		handlers["cmd3"] = [](xx::HttpContext& request, xx::HttpResponse& response)->int {
			return response.SendHtmlBody(R"--(
todo3</br>
<a href="/">home</a>
)--");
		};
	}
};


struct MyHttpServer : xx::SimpleHttpServer {
	using BaseType = xx::SimpleHttpServer;

	MyHtmlHandler hh;

	MyHttpServer() {
		this->onReceiveHttp = [this](xx::HttpContext& request, xx::HttpResponse& response)->int {
			return hh(request, response);
		};
	}

	inline virtual void OnAccept(xx::Epoll::Peer_r pr, int const& listenIndex) override {
		xx::CoutN(pr->ip, " connected.");
		this->BaseType::OnAccept(pr, listenIndex);
	}

	inline virtual void OnDisconnect(xx::Epoll::Peer_r pr) override {
		xx::CoutN(pr->ip, " disconnected.");
		this->BaseType::OnDisconnect(pr);
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
