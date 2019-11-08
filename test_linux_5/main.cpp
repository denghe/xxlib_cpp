#include "xx_epoll_http.h"
#include "xx_html.h"

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
<p><a href="/cmd1">test form get</a></p>
<p><a href="/cmd3">test form post</a></p>
<p><a href="/cmd5">test table</a></p>
<p><a href="/cmd6">test a</a></p>
)--");
		};

		handlers["cmd1"] = [](xx::HttpContext& request, xx::HttpResponse& response)->int {
			return response.Send(response.prefixHtml, R"+-+(
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

		handlers["cmd2"] = [](xx::HttpContext& request, xx::HttpResponse& response)->int {
			xx::Html::Document doc;
			auto&& body = doc.Add(xx::Html::Body::Create());
			body->Add(xx::Html::Paragrapth::Create("un = ", xx::HtmlEncode(request["un"])));
			body->Add(xx::Html::Paragrapth::Create("pw = ", xx::HtmlEncode(request["pw"])));
			body->Add(xx::Html::Paragrapth::Create("age = ", xx::HtmlEncode(request["age"])));
			body->Add(xx::Html::HyperLink::Create("home", "/"));
			return response.Send(response.prefixHtml, doc);
		};

		handlers["cmd3"] = [](xx::HttpContext& request, xx::HttpResponse& response)->int {
			xx::Html::Document doc;
			auto&& body = doc.Add(xx::Html::Body::Create());
			auto&& form = body->Add(xx::Html::Form::Create("/cmd4"));
			form->Add(xx::Html::Input::Create("username"));
			form->Add(xx::Html::Input::Create("password"));
			body->Add(xx::Html::HyperLink::Create("home", "/"));
			return response.Send(response.prefixHtml, doc);
		};

		handlers["cmd4"] = [](xx::HttpContext& request, xx::HttpResponse& response)->int {
			request.ParsePost();
			xx::Html::Document doc;
			auto&& body = doc.Add(xx::Html::Body::Create());
			body->Add(xx::Html::Paragrapth::Create("username = ", xx::HtmlEncode(request["username"])));
			body->Add(xx::Html::Paragrapth::Create("password = ", xx::HtmlEncode(request["password"])));
			body->Add(xx::Html::HyperLink::Create("home", "/"));
			return response.Send(response.prefixHtml, body);
		};

		handlers["cmd5"] = [](xx::HttpContext& request, xx::HttpResponse& response)->int {
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
			xx::Html::Document doc;
			auto&& body = doc.Add(xx::Html::Body::Create());
			body->Add(xx::Html::Table::Create(2, [&](int const& columnIndex, std::string& s) {
				switch (columnIndex) {
				case 0:
					xx::Append(s, "A::n");
					break;
				case 1:
					xx::Append(s, "A::s");
					break;
				}
				}, [&](int const& rowIndex, int const& columnIndex, std::string& s)->bool {
					switch (columnIndex) {
					case 0:
						xx::Append(s, as[rowIndex].n);
						break;
					case 1:
						xx::Append(s, as[rowIndex].s);
						break;
					}
					return rowIndex + 1 < (int)as.size();
				}));
			body->Add(xx::Html::HyperLink::Create("home", "/"));
			return response.Send(response.prefixHtml, doc);
		};

		handlers["cmd6"] = [](xx::HttpContext& request, xx::HttpResponse& response)->int {
			xx::Html::Document doc;
			auto&& body = doc.Add(xx::Html::Body::Create());
			body->Add(xx::Html::HyperLink::Create("中文", "/中文!哦哦哦?asdf=", xx::UrlEncode("汉汉<%' \">字字")));
			return response.Send(response.prefixHtml, doc);
		};

		handlers["中文!哦哦哦"] = [](xx::HttpContext& request, xx::HttpResponse& response)->int {
			xx::Html::Document doc;
			auto&& body = doc.Add(xx::Html::Body::Create());
			body->Add(xx::Html::Paragrapth::Create("queries:"));
			for (auto&& kv : request.queries) {
				body->Add(xx::Html::Paragrapth::Create(xx::HtmlEncode(kv.first), " : ", xx::HtmlEncode(kv.second)));
			}
			body->Add(xx::Html::HyperLink::Create("home", "/"));
			return response.Send(response.prefixHtml, doc);
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
