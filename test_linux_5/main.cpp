#include "xx_epoll.h"
#include "http_parser.h"

struct HttpParser;
struct HttpParserContext {
	HttpParserContext(HttpParser* parser) : parser(parser) {};
	HttpParserContext(HttpParserContext const&) = delete;
	HttpParserContext(HttpParserContext&&) = default;
	HttpParserContext& operator=(HttpParserContext const&) = delete;
	HttpParserContext& operator=(HttpParserContext&&) = default;

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

	// 头部所有键值对
	std::unordered_map<std::string, std::string> headers;

	// 当收到 key 时, 先往这 append. 出现 value 时再塞 headers
	std::string lastKey;

	// 指向最后一次塞入 headers 的 value 以便 append
	std::string* lastValue = nullptr;

	// 是否保持连接
	bool keepAlive = false;
};

struct HttpParser {
	// 指向 peer
	xx::Epoll::Peer_r pr;

	// 来自 libuv 作者的转换器及配置
	http_parser_settings parser_settings;
	http_parser parser;

	// 已接收到的 http 数据
	std::deque<HttpParserContext> ctxs;

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


	// 返回文本 / json 的通用 http 头
	inline static const std::string responseHeader_Json =
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/plain;charset=utf-8\r\n"		// text/json
		"Content-Length: "
		;

	HttpParser(xx::Epoll::Peer_r pr) : pr(pr) {
	
		http_parser_init(&parser, HTTP_BOTH);
		parser.data = this;

		http_parser_settings_init(&parser_settings);
		parser_settings.on_message_begin = [](http_parser* parser) noexcept {
			auto p = (HttpParser*)parser->data;
			parser->data = &p->ctxs.emplace_back(p);	// data 指向具体数据块 开始填充
			return 0;
		};
		parser_settings.on_url = [](http_parser* parser, const char* buf, std::size_t length) noexcept {
			((HttpParserContext*)parser->data)->url.append(buf, length);
			return 0;
		};
		parser_settings.on_status = [](http_parser* parser, const char* buf, std::size_t length) noexcept {
			((HttpParserContext*)parser->data)->status.append(buf, length);
			return 0;
		};
		parser_settings.on_header_field = [](http_parser* parser, const char* buf, std::size_t length) noexcept {
			auto c = (HttpParserContext*)parser->data;
			if (c->lastValue) {
				c->lastValue = nullptr;
			}
			c->lastKey.append(buf, length);
			return 0;
		};
		parser_settings.on_header_value = [](http_parser* parser, const char* buf, std::size_t length) noexcept {
			auto c = (HttpParserContext*)parser->data;
			if (!c->lastValue) {
				auto&& r = c->headers[c->lastKey];
				c->lastValue = &r;
			}
			c->lastValue->append(buf, length);
			return 0;
		};
		parser_settings.on_headers_complete = [](http_parser* parser) noexcept {
			auto c = (HttpParserContext*)parser->data;
			c->lastValue = nullptr;
			c->method = http_method_str((http_method)parser->method);
			c->keepAlive = http_should_keep_alive(parser);
			return 0;
		};
		parser_settings.on_body = [](http_parser* parser, const char* buf, std::size_t length) noexcept {
			((HttpParserContext*)parser->data)->body.append(buf, length);
			return 0;
		};
		parser_settings.on_message_complete = [](http_parser* parser) noexcept {
			auto c = (HttpParserContext*)parser->data;
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

	// 快捷发回 json 内容响应
	inline int SendHttpResponse_Text(char const* const& buf, std::size_t const& len) noexcept {
		if (!pr) return -1;

		// calc buf max len
		auto cap = /* http header */responseHeader_Json.size() + /* len */20 + /*\r\n\r\n*/4 + /* body */len + /* refs */8;

		// alloc memory
		auto data = (char*)::malloc(cap);
		std::size_t dataLen = 0;

		// append http header
		::memcpy(data + dataLen, responseHeader_Json.data(), responseHeader_Json.size());
		dataLen += responseHeader_Json.size();

		// append len
		auto&& lenStr = std::to_string(len);
		::memcpy(data + dataLen, lenStr.data(), lenStr.size());
		dataLen += lenStr.size();

		// append \r\n\r\n
		::memcpy(data + dataLen, "\r\n\r\n", 4);
		dataLen += 4;

		// append body
		::memcpy(data + dataLen, buf, len);
		dataLen += len;

		// send
		return pr->Send(xx::Epoll::Buf(dataLen, data));
	}

	inline int SendHttpResponse_Text(std::string const& s) noexcept {
		return SendHttpResponse_Text(s.data(), s.size());
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
		pr->userData = new HttpParser(pr);
	}
	inline virtual int OnReceive(xx::Epoll::Peer_r pr) override {
		auto&& hp = (HttpParser*)pr->userData;
		if (auto&& r = hp->Input((char*)pr->recv.buf, pr->recv.len)) return r;
		auto&& count = hp->GetFinishedCtxsCount();
		while(count) {
			auto&& h = hp->ctxs.front();
			
			// todo: handle h
			xx::CoutN(threadId, " OnReceive http text: id = ", pr->id, ", fd = ", pr->sockFD, ", ip = ", pr->ip, ", url = ", h.url);
			hp->SendHttpResponse_Text("hi~~~");

			hp->ctxs.pop_front();
			--count;
		}
		return 0;
	}
	inline virtual void OnDisconnect(xx::Epoll::Peer_r pr) override {
		xx::CoutN(threadId, " OnDisconnect: id = ", pr->id, ", ip = ", pr->ip);
		delete (HttpParser*)pr->userData;
	}
};
int main() {
	xx::IgnoreSignal();
	auto&& server = std::make_unique<SimpleHttpServer>();
	int r = server->Listen(12345);
	assert(!r);
	server->Run(1);
	return 0;
}
