#include "xx_epoll.h"
#include "http_parser.h"

struct HttpParser;
struct HttpContext {
	HttpContext(HttpParser* parser) : parser(parser) {};
	HttpContext(HttpContext const&) = delete;
	HttpContext(HttpContext&&) = default;
	HttpContext& operator=(HttpContext const&) = delete;
	HttpContext& operator=(HttpContext&&) = default;

	// 指向 parser
	HttpParser* parser = nullptr;

	// GET / POST / ...
	std::string method;

	// 正文
	std::string body;

	// 原始 url 串( 未 urldecode 解码 )
	std::string url;

	// 原始 status 串
	std::string status;

	// 是否保持连接的标识
	bool keepAlive = false;

	// 头部所有键值对
	std::unordered_map<std::string, std::string> headers;


	friend struct HttpParser;
protected:
	// 当收到 key 时, 先往这 append. 出现 value 时再塞 headers
	std::string lastKey;

	// 指向最后一次塞入 headers 的 value 以便 append
	std::string* lastValue = nullptr;
};

struct HttpParser {
	// 来自 libuv 作者的转换器及配置
	http_parser_settings parser_settings;
	http_parser parser;

	// 已接收到的 http 数据
	std::deque<HttpContext> ctxs;

	// 获取已接收完整的 http 数据个数
	std::size_t GetFinishedCtxsCount() {
		return ctxs.size() - (parser.data == this ? 0 : 1);
	}


	// url decode 后的结果 url 串, 同时也是 url parse 后的容器, 不可以修改内容
	std::string urlDecoded;

	// ParseUrl 后将填充下面三个属性. 这些指针都指向 std::string 内部, 可能失效, 需要注意访问时机
	char* path = nullptr;
	std::vector<std::pair<char*, char*>> queries;	// 键值对
	char* fragment = nullptr;


	HttpParser() {

		http_parser_init(&parser, HTTP_BOTH);
		parser.data = this;

		http_parser_settings_init(&parser_settings);
		parser_settings.on_message_begin = [](http_parser* parser) noexcept {
			auto p = (HttpParser*)parser->data;
			parser->data = &p->ctxs.emplace_back(p);	// data 指向具体数据块 开始填充
			return 0;
		};
		parser_settings.on_url = [](http_parser* parser, const char* buf, std::size_t length) noexcept {
			((HttpContext*)parser->data)->url.append(buf, length);
			return 0;
		};
		parser_settings.on_status = [](http_parser* parser, const char* buf, std::size_t length) noexcept {
			((HttpContext*)parser->data)->status.append(buf, length);
			return 0;
		};
		parser_settings.on_header_field = [](http_parser* parser, const char* buf, std::size_t length) noexcept {
			auto c = (HttpContext*)parser->data;
			if (c->lastValue) {
				c->lastValue = nullptr;
			}
			c->lastKey.append(buf, length);
			return 0;
		};
		parser_settings.on_header_value = [](http_parser* parser, const char* buf, std::size_t length) noexcept {
			auto c = (HttpContext*)parser->data;
			if (!c->lastValue) {
				auto&& r = c->headers[c->lastKey];
				c->lastValue = &r;
			}
			c->lastValue->append(buf, length);
			return 0;
		};
		parser_settings.on_headers_complete = [](http_parser* parser) noexcept {
			auto c = (HttpContext*)parser->data;
			c->lastValue = nullptr;
			c->method = http_method_str((http_method)parser->method);
			c->keepAlive = http_should_keep_alive(parser);
			return 0;
		};
		parser_settings.on_body = [](http_parser* parser, const char* buf, std::size_t length) noexcept {
			((HttpContext*)parser->data)->body.append(buf, length);
			return 0;
		};
		parser_settings.on_message_complete = [](http_parser* parser) noexcept {
			auto c = (HttpContext*)parser->data;
			parser->data = c->parser;			// data 指向数据块容器 填充结束
			return 0;
		};
		parser_settings.on_chunk_header = [](http_parser* parser) noexcept {
			return 0;
		};
		parser_settings.on_chunk_complete = [](http_parser* parser) noexcept {
			return 0;
		};
	}


	// 输入数据, 解析完成的 http 数据将压入 ctxs. 用 GetFinishedCtxsCount() 来拿正确长度. 
	inline int Input(char const* const& buf, std::size_t const& len) {
		auto&& parsedLen = http_parser_execute(&parser, &parser_settings, buf, len);
		if (parsedLen < len) {
			return parser.http_errno;			// http_errno_description( (http_errno)xxx )
		}
		return 0;
	}
};

struct SimpleHttpServer : xx::Epoll::Instance {

	inline virtual void OnAccept(xx::Epoll::Peer_r pr, int const& listenIndex) override {
		xx::CoutN(threadId, " OnAccept: id = ", pr->id, ", fd = ", pr->sockFD, ", ip = ", pr->ip);
		pr->userData = new HttpParser();
	}
	inline virtual void OnDisconnect(xx::Epoll::Peer_r pr) override {
		xx::CoutN(threadId, " OnDisconnect: id = ", pr->id, ", ip = ", pr->ip);
		delete (HttpParser*)pr->userData;
	}

	inline int SendResponse(std::string const& prefix, char const* const& buf, std::size_t const& len) noexcept {
		if (!peer) return -1;

		// calc buf max len
		auto cap = /* partial http header */prefix.size() + /* len */20 + /*\r\n\r\n*/4 + /* body */len + /* refs */8;

		// alloc memory
		auto data = (char*)::malloc(cap);
		std::size_t dataLen = 0;

		// append partial http header
		::memcpy(data + dataLen, prefix.data(), prefix.size());
		dataLen += prefix.size();

		// append len
		auto&& lenStr = std::to_string(len);
		::memcpy(data + dataLen, lenStr.data(), lenStr.size());
		dataLen += lenStr.size();

		// append \r\n\r\n
		::memcpy(data + dataLen, "\r\n\r\n", 4);
		dataLen += 4;

		// append content
		::memcpy(data + dataLen, buf, len);
		dataLen += len;

		// send
		return peer->Send(xx::Epoll::Buf(dataLen, data));
	}

	inline virtual int OnReceive(xx::Epoll::Peer_r pr) override {
		auto&& hp = (HttpParser*)pr->userData;
		if (auto&& r = hp->Input((char*)pr->recv.buf, pr->recv.len)) return r;

		this->peer = pr;
		auto&& count = hp->GetFinishedCtxsCount();
		while (count) {
			this->request = &hp->ctxs.front();
			ParseUrl();

			auto&& iter = handlers.find(path);
			if (iter == handlers.end()) {
				SendResponse_404("<html><body>404: the page not found!!!</body></html>");
			}
			else {

				if (iter->second()) {
					SendResponse_404("<html><body>bad request!</body></html>");
				}
			}

			hp->ctxs.pop_front();
			--count;
		}
		return 0;
	}


	// 网址 path : 处理函数 映射填充到此
	std::unordered_map<std::string, std::function<int()>> handlers;


	// 下列 fields 都是临时有效, 于 call handler 前填充

	// 公用输出拼接容器. 调用 handle 前清空
	std::string response;

	// 指向当前 http 请求的连接
	xx::Epoll::Peer_r peer;

	// 指向当前 http 请求的上下文
	HttpContext* request = nullptr;


	// url decode 数据容器, 不可以修改内容( queries 里面的 char* 会指向这里 )
	std::string urlDecoded;

	// 前面不带 /, 不含 ? 和 #
	std::string path;

	// ?a=1&b=2....参数部分的键值对
	std::vector<std::pair<char*, char*>> queries;

	// #xxxxxx 部分
	char* fragment = nullptr;



	// 会 urldecode 并 填充 path, queries, fragment. 需要手动调用
	inline void ParseUrl() noexcept {
		auto&& url = request->url;
		urlDecoded.reserve(url.size());
		urlDecoded.clear();
		UrlDecode(url, urlDecoded);
		auto u = (char*)urlDecoded.c_str();

		// 从后往前扫
		fragment = FindAndTerminate(u, '#');
		auto q = FindAndTerminate(u, '?');
		if (auto&& s = FindAndTerminate(u, '/')) {
			path = s;
		}
		else {
			path.clear();
		}

		queries.clear();
		if (!q || '\0' == *q) return;
		queries.reserve(16);
		int i = 0;
		queries.resize(i + 1);
		queries[i++].first = q;
		while ((q = strchr(q, '&'))) {
			queries.reserve(i + 1);
			queries.resize(i + 1);
			*q = '\0';
			queries[i].first = ++q;
			queries[i].second = nullptr;

			if (i && (queries[i - 1].second = strchr(queries[i - 1].first, '='))) {
				*(queries[i - 1].second)++ = '\0';
			}
			i++;
		}
		if ((queries[i - 1].second = strchr(queries[i - 1].first, '='))) {
			*(queries[i - 1].second)++ = '\0';
		}
	}

	inline static char* FindAndTerminate(char* s, char const& c) noexcept {
		s = strchr(s, c);
		if (!s) return nullptr;
		*s = '\0';
		return s + 1;
	}

	inline static int FromHex(uint8_t const& c) noexcept {
		if (c >= 'A' && c <= 'Z') return c - 'A' + 10;
		else if (c >= 'a' && c <= 'z') return c - 'a' + 10;
		else if (c >= '0' && c <= '9') return c - '0';
		else return 0;
	}

	inline static void UrlDecode(std::string const& src, std::string& dst) noexcept {
		for (std::size_t i = 0; i < src.size(); i++) {
			if (src[i] == '+') {
				dst += ' ';
			}
			else if (src[i] == '%') {
				auto high = FromHex(src[++i]);
				auto low = FromHex(src[++i]);
				dst += ((char)(uint8_t)(high * 16 + low));
			}
			else dst += src[i];
		}
	}




	inline int SendResponse_Json(std::string const& s) noexcept {
		return SendResponse(
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: text/plain;charset=utf-8\r\n"
			"Connection: close\r\n"
			"Content-Length: "
			, s.data(), s.size());
	}
	inline int SendResponse_Text(std::string const& s) noexcept {
		return SendResponse(
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: text/plain;charset=utf-8\r\n"
			"Connection: close\r\n"
			"Content-Length: "
			, s.data(), s.size());
	}
	inline int SendResponse_Html(std::string const& s) noexcept {
		return SendResponse(
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: text/html;charset=utf-8\r\n"
			"Connection: close\r\n"
			"Content-Length: "
			, s.data(), s.size());
	}
	inline int SendResponse_404(std::string const& s) noexcept {
		return SendResponse(
			"HTTP/1.1 404 Not Found\r\n"
			"Content-Type: text/plain;charset=utf-8\r\n"
			"Connection: close\r\n"
			"Content-Length: "
			, s.data(), s.size());
	}


	// 简易拼接返回内容
	inline int SendResponse_HtmlBody(std::string const& body) {
		response.clear();
		xx::Append(response, "<html><body>", body, "</body></html>");
		return SendResponse_Html(response);
	}

	// todo: more helper funcs
};

struct MyHttpServer : SimpleHttpServer {
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
