#include "xx_uv_http.h"
// todo: 超时检测机制	// todo: http dialer

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


int main() {
	xx::IgnoreSignal();

	xx::Uv uv;
	xx::UvHttpListener listener(uv, "0.0.0.0", 12345);

	MyHtmlHandler hh;

	listener.onAccept = [&hh](xx::UvHttpPeer_s peer) {
		peer->onReceiveHttp = [peer, &hh](xx::HttpContext& request, xx::HttpResponse& response)->int {
			return hh(request, response);
		};

		peer->onDisconnect = [peer] {
			xx::CoutN(peer->ip, " disconnected.");
		};

		xx::CoutN(peer->ip, " connected.");
	};

	uv.Run();
	return 0;
}
