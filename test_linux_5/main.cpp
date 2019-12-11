#include "xx_epoll_http.h"

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

		// 直接输出模式，zero copy, 性能最快
		handlers[""] = [](xx::HttpContext& q, xx::HttpResponse& r)->int {
			//char const str[] = R"--(<html><body>hello world!</body></html>)--";
			char const str[] = R"--(
<html><body>
<p><a href="/cmd1">test form get</a></p>
<p><a href="/cmd3">test form post</a></p>
<p><a href="/cmd5">test table</a></p>
<p><a href="/cmd6">test a</a></p>
</body></html>
)--";
			return r.onSend(r.prefixHtml, str, sizeof(str));
		};

		// 借助 r.text 抠洞 拼接内容，性能还行
		handlers["cmd1"] = [](xx::HttpContext& q, xx::HttpResponse& r)->int {
			return r.Send(r.prefixHtml, R"+-+(
<html><body>
<p>ticks = )+-+", xx::NowEpoch10m(), R"+-+(</p>
<p>ticks = )+-+", xx::NowEpoch10m(), R"+-+(</p>
<p>ticks = )+-+", xx::NowEpoch10m(), R"+-+(</p>
<form method="get" action="/cmd2">
  <p>username: <input type="text" name="un" /></p>
  <p>password: <input type="text" name="pw" /></p>
  <p>age: <input type="number" name="age" /></p>
  <input type="submit" value="Submit" />
</form>
<a href="/">home</a>
</body></html>
)+-+");
		};

		// 借助 r.text 更灵活的拼接内容，性能正常
		handlers["cmd2"] = [](xx::HttpContext& q, xx::HttpResponse& r)->int {
			{
				auto&& hb = r.Scope("<html><body>", "</body></html>");
				r.P("un = ", xx::HtmlEncode(q["un"]));
				r.P("pw = ", xx::HtmlEncode(q["pw"]));
				r.P("age = ", xx::HtmlEncode(q["age"]));
				r.A("home", "/");
			}
			return r.Send();
		};

		handlers["cmd3"] = [](xx::HttpContext& q, xx::HttpResponse& r)->int {
			{
				auto&& hb = r.Scope("<html><body>", "</body></html>");
				r.FormBegin("/cmd4");
				r.Input("username");
				r.Input("password");
				r.FormEnd("Submit");
				r.A("home", "/");
			}
			return r.Send();
		};

		handlers["cmd4"] = [](xx::HttpContext& q, xx::HttpResponse& r)->int {
			q.ParsePost();
			{
				auto&& hb = r.Scope("<html><body>", "</body></html>");
				r.P("username = ", xx::HtmlEncode(q["username"]));
				r.P("password = ", xx::HtmlEncode(q["password"]));
				r.A("home", "/");
			}
			return r.Send();
		};

		handlers["cmd5"] = [](xx::HttpContext& q, xx::HttpResponse& r)->int {
			// 伪造点数据
			struct A {
				int n = 0;
				std::string s;
			};
			std::vector<A> as{
				  { 0, "aaa" }
				, { 1, "bbbbb" }
				, { 2, "cccccc" }
				, { 3, "dddddddd" }
				, { 4, "ee" }
			};

			{
				auto&& hb = r.Scope("<html><body>", "</body></html>");
				r.TableBegin("A::n", "a::s");
				for (auto&& a : as) { r.TableRow(a.n, a.s); }
				r.TableEnd();
				r.A("home", "/");
			}
			return r.Send();
		};

		handlers["cmd6"] = [](xx::HttpContext& q, xx::HttpResponse& r)->int {
			{
				auto&& hb = r.Scope("<html><body>", "</body></html>");
				r.A("中文", "/中文!哦哦哦?asdf=", xx::UrlEncode("汉汉<%' \">字字"));
			}
			return r.Send();
		};

		handlers["中文!哦哦哦"] = [](xx::HttpContext& q, xx::HttpResponse& r)->int {
			{
				auto&& hb = r.Scope("<html><body>", "</body></html>");
				r.P("queries:");
				for (auto&& kv : q.queries) {
					r.P(xx::HtmlEncode(kv.first), " : ", xx::HtmlEncode(kv.second));
				}
				r.A("home", "/");
			}
			return r.Send();
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

	//inline virtual void OnAccept(xx::Epoll::Peer_r pr, int const& listenIndex) override {
	//	xx::CoutN(pr->ip, " connected.");
	//	this->BaseType::OnAccept(pr, listenIndex);
	//}

	//inline virtual void OnDisconnect(xx::Epoll::Peer_r pr) override {
	//	xx::CoutN(pr->ip, " disconnected.");
	//	this->BaseType::OnDisconnect(pr);
	//}
};

int main() {
	xx::IgnoreSignal();



	auto&& s = std::make_unique<MyHttpServer>();
	int r = s->Listen(12312);
	assert(!r);

	//auto fd = s->listenFDs[0];
	//std::vector<std::thread> threads;
	//for (int i = 0; i < 5; ++i) {
	//	threads.emplace_back([fd, i] {
	//		auto&& s = std::make_unique<MyHttpServer>();
	//		int r = s->ListenFD(fd);
	//		assert(!r);
	//		s->threadId = i + 1;
	//		xx::CoutN("thread:", i + 1);
	//		s->Run(1);
	//		}).detach();
	//}

	s->Run(1);
	return 0;
}
