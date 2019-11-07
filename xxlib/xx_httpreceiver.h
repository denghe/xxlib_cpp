#pragma once
#include "xx_bbuffer.h"
#include "http_parser.h"

namespace xx {
	struct HttpReceiver;
	struct SimpleHttpServer;
	struct HttpContext {
		HttpContext(HttpReceiver* parser) : parser(parser) {};
		HttpContext(HttpContext const&) = delete;
		HttpContext(HttpContext&&) = default;
		HttpContext& operator=(HttpContext const&) = delete;
		HttpContext& operator=(HttpContext&&) = default;

		// 指向 parser
		HttpReceiver* parser = nullptr;

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



		// 下面 3 个 field 在执行 ParseUrl 之后被填充

		// 前面不带 /, 不含 ? 和 #
		std::string path;

		// ?a=1&b=2....参数部分的键值对
		std::vector<std::pair<char*, char*>> queries;

		// #xxxxxx 部分
		char* fragment = nullptr;



		friend struct HttpReceiver;
		friend struct SimpleHttpServer;
	protected:
		// 当收到 key 时, 先往这 append. 出现 value 时再塞 headers
		std::string lastKey;

		// 指向最后一次塞入 headers 的 value 以便 append
		std::string* lastValue = nullptr;

		// url decode 数据容器, 不可以修改内容( queries 里面的 char* 会指向这里 )
		std::string tmp;

		// 会 urldecode 并 填充 path, queries, fragment. 需要手动调用
		inline void ParseUrl() noexcept {
			tmp.reserve(url.size());
			tmp.clear();
			UrlDecode(url, tmp);
			auto u = (char*)tmp.c_str();

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
	};

	struct HttpReceiver {
		// 来自 libuv 作者的转换器及配置
		http_parser_settings parser_settings;
		http_parser parser;

		// 已接收到的 http 数据
		std::deque<HttpContext> ctxs;

		// 获取已接收完整的 http 数据个数
		std::size_t GetFinishedCtxsCount() {
			return ctxs.size() - (parser.data == this ? 0 : 1);
		}

		HttpReceiver() {
			http_parser_init(&parser, HTTP_BOTH);
			parser.data = this;

			http_parser_settings_init(&parser_settings);
			parser_settings.on_message_begin = [](http_parser* parser) noexcept {
				auto p = (HttpReceiver*)parser->data;
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
				//xx::CoutN(http_errno_description((http_errno)parser.http_errno));
				return parser.http_errno;
			}
			return 0;
		}
	};
}
