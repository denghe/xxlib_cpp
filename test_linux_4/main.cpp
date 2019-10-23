#include "xx_uv.h"
#include "http_parser.h"

// todo: 需要专门写独立的 listener & peer 只需要支持 tcp 并且收包逻辑简单粗暴.

namespace xx {
	// http 服务 peer 基类
	// 基类的 SendXxxx, onReceiveXxxxx 不要用
	struct UvHttpPeer : UvPeer {

		// 来自 libuv 作者的转换器及配置
		http_parser_settings parser_settings;
		http_parser parser;

		// GET / POST / ...
		std::string method;

		// 头部所有键值对
		std::unordered_map<std::string, std::string> headers;

		// 是否保持连接
		bool keepAlive = false;

		// 正文
		std::string body;

		// 原始 url 串( 未 urldecode 解码 )
		std::string url;

		// url decode 后的结果 url 串, 同时也是 url parse 后的容器, 不可以修改内容
		std::string urlDecoded;

		// ParseUrl 后将填充下面三个属性. 这些指针都指向 std::string 内部, 可能失效, 需要注意访问时机
		char* path = nullptr;
		std::vector<std::pair<char*, char*>> queries;	// 键值对
		char* fragment = nullptr;

		// 原始 status 串
		std::string status;

		// 当收到 key 时, 先往这 append. 出现 value 时再塞 headers
		std::string lastKey;

		// 指向最后一次塞入 headers 的 value 以便 append
		std::string* lastValue = nullptr;

		// 成功接收完一段信息时的回调.
		std::function<void()> OnReceiveHttp = []() noexcept {};

		// 接收出错回调. 接着会发生 Release
		std::function<void(uint32_t errorNumber, char const* errorMessage)> OnError;

		inline static const std::string responseHeader_Json =
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: text/plain;charset=utf-8\r\n"		// text/json
			"Content-Length: "
			;

		UvHttpPeer(Uv& uv) : UvPeer(uv) {
			parser.data = this;
			http_parser_init(&parser, HTTP_REQUEST);									// too: 挪到函数中去? 以便作为 client 用? HTTP_RESPONSE

			http_parser_settings_init(&parser_settings);
			parser_settings.on_message_begin = [](http_parser* parser) noexcept {
				auto self = (UvHttpPeer*)parser->data;
				self->method.clear();
				self->headers.clear();
				self->keepAlive = false;
				self->body.clear();
				self->url.clear();
				self->urlDecoded.clear();
				self->status.clear();
				self->lastKey.clear();
				self->lastValue = nullptr;
				return 0;
			};
			parser_settings.on_url = [](http_parser* parser, const char* buf, std::size_t length) noexcept {
				((UvHttpPeer*)parser->data)->url.append(buf, length);
				return 0;
			};
			parser_settings.on_status = [](http_parser* parser, const char* buf, std::size_t length) noexcept {
				((UvHttpPeer*)parser->data)->status.append(buf, length);
				return 0;
			};
			parser_settings.on_header_field = [](http_parser* parser, const char* buf, std::size_t length) noexcept {
				auto self = (UvHttpPeer*)parser->data;
				if (self->lastValue) {
					self->lastValue = nullptr;
				}
				self->lastKey.append(buf, length);
				return 0;
			};
			parser_settings.on_header_value = [](http_parser* parser, const char* buf, std::size_t length) noexcept {
				auto self = (UvHttpPeer*)parser->data;
				if (!self->lastValue) {
					auto&& r = self->headers[self->lastKey];
					self->lastValue = &r;
				}
				self->lastValue->append(buf, length);
				return 0;
			};
			parser_settings.on_headers_complete = [](http_parser* parser) noexcept {
				auto self = (UvHttpPeer*)parser->data;
				self->lastValue = nullptr;
				self->method = http_method_str((http_method)parser->method);
				self->keepAlive = http_should_keep_alive(parser);
				return 0;
			};
			parser_settings.on_body = [](http_parser* parser, const char* buf, std::size_t length) noexcept {
				((UvHttpPeer*)parser->data)->body.append(buf, length);
				return 0;
			};
			parser_settings.on_message_complete = [](http_parser* parser) noexcept {
				auto self = (UvHttpPeer*)parser->data;
				self->OnReceiveHttp();
				return 0;
			};
			parser_settings.on_chunk_header = [](http_parser* parser) noexcept { return 0; };
			parser_settings.on_chunk_complete = [](http_parser* parser) noexcept { return 0; };
		}

		// 发送 buf
		inline void SendHttpResponse(char const* const& bufPtr, std::size_t const& len) noexcept {
			// prepare
			auto&& bb = uv.sendBB;
			bb.Clear();

			// write prefix
			bb.AddRange((uint8_t*)responseHeader_Json.data(), responseHeader_Json.size());

			// write len\r\n\r\n
			lastKey.clear();
			lastKey += len;
			bb.AddRange((uint8_t*)lastKey.data(), lastKey.size());
			bb.AddRange((uint8_t*)"\r\n\r\n", 4);

			// write body
			bb.AddRange((uint8_t*)bufPtr, len);

			// send data
			SendDirect(bb.buf, bb.len);
		}

		// 会 urldecode 并 填充 path, queries, fragment
		inline void ParseUrl() noexcept {
			urlDecoded.reserve(url.size());
			UrlDecode(url, urlDecoded);
			auto u = (char*)urlDecoded.c_str();

			// 从后往前扫
			fragment = FindAndTerminate(u, '#');
			auto q = FindAndTerminate(u, '?');
			path = FindAndTerminate(u, '/');

			queries.clear();
			if (!q || '\0' == *q) return;
			queries.reserve(16);
			int i = 0;
			queries.resize(i + 1);
			queries[i++].first = q;
			while (q = strchr(q, '&')) {
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
			if (queries[i - 1].second = strchr(queries[i - 1].first, '=')) {
				*(queries[i - 1].second)++ = '\0';
			}
		}

		inline static char* FindAndTerminate(char* s, char const& c) noexcept {
			s = strchr(s, c);
			if (!s) return nullptr;
			*s = '\0';
			return s + 1;
		}

		inline static uint8_t FromHex(uint8_t const& c) noexcept {
			if (c >= 'A' && c <= 'Z') return c - 'A' + 10;
			else if (c >= 'a' && c <= 'z') return c - 'a' + 10;
			else if (c >= '0' && c <= '9') return c - '0';
			else return 0;
		}

		inline static void UrlDecode(std::string& src, std::string& dst) noexcept {
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

		// todo: 下面代码在解析完成时
		/*
		void xx::UvHttpPeer::ReceiveImpl(char const* const& bufPtr, int const& len) noexcept
		{
			if (rawData)
			{
				rawData->AddRange(bufPtr, len);	// 如果粘包, 尾部可能会多点东西出来, 当前不方便去除
			}
			auto vn = memHeader().versionNumber;
			auto parsed = http_parser_execute(parser, parser_settings, bufPtr, len);
			if (IsReleased(vn)) return;
			if ((int)parsed < len)
			{
				if (OnError)
				{
					OnError(parser->http_errno, http_errno_description((http_errno)parser->http_errno));
					if (IsReleased(vn)) return;
				}
				Release();
			}
		}*/
	};
}

int main() {
	xx::IgnoreSignal();
	return 0;
}
