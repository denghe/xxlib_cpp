#pragma once
#include "xx_bbuffer.h"
#include "http_parser.h"

namespace xx {
	struct HttpReceiver;
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

		// #xxxxxx 部分
		char* fragment = nullptr;

		// ?a=1&b=2....参数部分的键值对
		std::vector<std::pair<char*, char*>> queries;

		// POST 模式 body 内容 a=1&b=2....参数部分的键值对
		std::vector<std::pair<char*, char*>> posts;


		// 依次在 queries 和 posts 中查找, 找到后立马返回. 未找到则返回 nullptr
		inline char const* operator[](char const* const& key) const noexcept {
			for (auto&& kv : queries) {
				if (!strcmp(kv.first, key)) return kv.second;
			}
			for (auto&& kv : posts) {
				if (!strcmp(kv.first, key)) return kv.second;
			}
			return nullptr;
		}


		// 从 queries 查找 queryKey 并转换数据类型填充 value
		template<typename T>
		inline bool TryParse(std::vector<std::pair<char*, char*>> const& kvs, char const* const& key, T& value) {
			for (auto&& item : kvs) {
				if (!strcmp(key, item.first)) {
					return xx::TryParse(item.second, value);
				}
			}
			return false;
		}

		// 从 queries 下标定位 并转换数据类型填充 value
		template<typename T>
		inline bool TryParse(std::vector<std::pair<char*, char*>> const& kvs, size_t const& index, T& value) {
			if (index >= kvs.size()) return false;
			return xx::TryParse(kvs[index].second, value);
		}

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

			FillQueries(queries, q);
		}

		// 会 解析 body 填充 posts. 需要手动调用
		inline void ParsePost() noexcept {
			tmp = body;
			FillQueries(posts, (char*)tmp.c_str());
		}

		friend struct HttpReceiver;
	protected:
		// 当收到 key 时, 先往这 append. 出现 value 时再塞 headers
		std::string lastKey;

		// 指向最后一次塞入 headers 的 value 以便 append
		std::string* lastValue = nullptr;

		// url decode 数据容器, 不可以修改内容( queries 里面的 char* 会指向这里 )
		std::string tmp;

		// url decode 数据容器, 不可以修改内容( posts 里面的 char* 会指向这里 )
		std::string tmp2;

		inline void FillQueries(std::vector<std::pair<char*, char*>>& kvs, char* q) noexcept {
			kvs.clear();
			if (!q || '\0' == *q) return;
			kvs.reserve(16);
			int i = 0;
			kvs.resize(i + 1);
			kvs[i++].first = q;
			while ((q = strchr(q, '&'))) {
				kvs.reserve(i + 1);
				kvs.resize(i + 1);
				*q = '\0';
				kvs[i].first = ++q;
				kvs[i].second = nullptr;

				if (i && (kvs[i - 1].second = strchr(kvs[i - 1].first, '='))) {
					*(kvs[i - 1].second)++ = '\0';
				}
				i++;
			}
			if ((kvs[i - 1].second = strchr(kvs[i - 1].first, '='))) {
				*(kvs[i - 1].second)++ = '\0';
			}
		}

		inline static char* FindAndTerminate(char* s, char const& c) noexcept {
			s = strchr(s, c);
			if (!s) return nullptr;
			*s = '\0';
			return s + 1;
		}
	};

	// 靠析构来写入 foot
	struct HtmlScope {
		HtmlScope(HtmlScope const&) = delete;
		HtmlScope(HtmlScope&&) = default;
		HtmlScope& operator=(HtmlScope const&) = delete;
		HtmlScope& operator=(HtmlScope&&) = default;

		std::string& tmp;
		std::string foot;

		HtmlScope(std::string& tmp, char const* const& head, char const* const& foot = nullptr)
			: tmp(tmp) {
			if (foot) {
				xx::Append(tmp, head);
				this->foot = foot;
			}
			else {
				xx::Append(tmp, "<", head, ">");
				xx::Append(this->foot, "</", head, ">");
			}
		}

		~HtmlScope() {
			xx::Append(tmp, foot);
		}
	};

	// 输出容器 & 辅助输出的拼接函数
	struct HttpResponse {
		HttpResponse() = default;
		HttpResponse(HttpResponse const&) = delete;
		HttpResponse(HttpResponse&&) = default;
		HttpResponse& operator=(HttpResponse const&) = delete;
		HttpResponse& operator=(HttpResponse&&) = default;

		// 公用输出拼接容器( 同一连接 跨请求, 原则上只能追加 不可清空, 否则可能破坏上个请求的输出 )
		std::string output;

		
		inline static std::string prefixText =
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: text/plain;charset=utf-8\r\n"
			"Connection: keep-alive\r\n"
			"Content-Length: ";

		inline static std::string prefixHtml =
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: text/html;charset=utf-8\r\n"
			"Connection: keep-alive\r\n"
			"Content-Length: ";

		inline static std::string prefix404 =
			"HTTP/1.1 404 Not Found\r\n"
			"Content-Type: text/html;charset=utf-8\r\n"
			"Connection: keep-alive\r\n"
			"Content-Length: ";


		// 创建一个会在析构时自动填充 </ tag> 的元素
		template<typename ...Args>
		inline HtmlScope Scope(char const* const& head, char const* const& foot = nullptr) {
			return HtmlScope(output, head, foot);
		}

		// 向 output 追加 <tag>...</tag> 或 <tag.../>  内容. 参数 tag 不能带 <>
		template<typename ...Args>
		inline void Tag(char const* const& tag, Args const& ... content) {
			if constexpr (sizeof...(content)) {
				xx::Append(output, "<", tag, ">", content..., "<", tag, "/>");
			}
			else {
				xx::Append(output, "<", tag, "/>");
			}
		}

		// 向 output 追加任意内容
		template<typename ...Args>
		inline void Append(Args const& ... content) {
			xx::Append(output, content...);
		}

		template<typename ...Args>
		inline void TableBegin(Args const& ... titles) {
			xx::Append(output, "<table border=\"1\">");
			xx::Append(output, "<thead><tr>");
			std::initializer_list<int> n{ ((xx::Append(output, "<th>", titles, "</th>")), 0)... };
			(void)(n);
			xx::Append(output, "</tr></thead><tbody>");
		}

		template<typename ...Args>
		inline void TableRow(Args const& ... columns) {
			xx::Append(output, "<tr>");
			std::initializer_list<int> n{ ((xx::Append(output, "<td>", columns, "</td>")), 0)... };
			(void)(n);
			xx::Append(output, "</tr>");
		}

		inline void TableEnd() {
			xx::Append(output, "</tbody></table>");
		}

		template<typename ...Args>
		inline void P(Args const& ... contents) {
			xx::Append(output, "<p>", contents..., "</p>");
		}

		template<typename ...Args>
		inline void A(char const* const& content, Args const& ... hrefs) {
			xx::Append(output, "<a href=\"", hrefs..., "\">", content, "</a>");
		}

		template<typename ...Args>
		inline void ABegin(Args const& ... hrefs) {
			xx::Append(output, "<a href=\"", hrefs..., "\">");
		}

		inline void AEnd() {
			xx::Append(output, "</a>");
		}

		template<typename ...Args>
		inline void FormBegin(Args const& ... actions) {
			xx::Append(output, "<form method=\"post\" action=\"", actions..., "\">");
		}

		template<typename ...Args>
		inline void FormEnd(Args const& ... submitTexts) {
			xx::Append(output, "<input type=\"submit\" value=\"", submitTexts...,"\" /></form>");
		}

		inline void Input(char const* const& title, char const* const& name = nullptr, char const* const& value = nullptr, char const* const& type = nullptr) {
			xx::Append(output, "<p>", (title ? title : name), ":<input type=\"", (type ? type : "text"), "\" name=\"", (name ? name : title), "\" value=\"", value, "\" /></p>");
		}



		// 下发 html( 由外部赋值 )
		std::function<int(std::string const& prefix, char const* const& buf, size_t const& len)> onSend;

		// 将 output 以 html 格式发出
		inline int Send() {
			return onSend(prefixHtml, output.data(), output.size());
		}

		// 会多一次复制但是方便拼接的下发 html 函数
		template<typename...Args>
		inline int Send(std::string const& prefix, Args const& ...args) noexcept {
			xx::Append(output, args...);
			return onSend(prefix, output.data(), output.size());
		}

		inline int SendHtml(std::string const& html) {
			return onSend(prefixHtml, html.data(), html.size());
		}

		inline int SendText(std::string const& text) {
			return onSend(prefixText, text.data(), text.size());
		}

		inline int SendJson(std::string const& json) {
			return onSend(prefixText, json.data(), json.size());
		}

		inline int Send404(std::string const& html) {
			return onSend(prefix404, html.data(), html.size());
		}

		inline int SendHtmlBody(std::string const& body) {
			return Send(prefixHtml, "<html><body>", body, "</body></html>");
		}

		inline int Send404Body(std::string const& body) {
			return Send(prefix404, "<html><body>", body, "</body></html>");
		}
	};


	struct HttpReceiver {
		HttpReceiver(HttpReceiver const&) = delete;
		HttpReceiver(HttpReceiver&&) = default;
		HttpReceiver& operator=(HttpReceiver const&) = delete;
		HttpReceiver& operator=(HttpReceiver&&) = default;

		// 来自 libuv 作者的转换器及配置
		http_parser_settings parser_settings;
		http_parser parser;

		// 已接收到的 http 数据
		std::deque<HttpContext> ctxs;

		// 获取已接收完整的 http 数据个数
		size_t GetFinishedCtxsCount() {
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
			parser_settings.on_url = [](http_parser* parser, const char* buf, size_t length) noexcept {
				((HttpContext*)parser->data)->url.append(buf, length);
				return 0;
			};
			parser_settings.on_status = [](http_parser* parser, const char* buf, size_t length) noexcept {
				((HttpContext*)parser->data)->status.append(buf, length);
				return 0;
			};
			parser_settings.on_header_field = [](http_parser* parser, const char* buf, size_t length) noexcept {
				auto c = (HttpContext*)parser->data;
				if (c->lastValue) {
					c->lastValue = nullptr;
				}
				c->lastKey.append(buf, length);
				return 0;
			};
			parser_settings.on_header_value = [](http_parser* parser, const char* buf, size_t length) noexcept {
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
			parser_settings.on_body = [](http_parser* parser, const char* buf, size_t length) noexcept {
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
		inline int Input(char const* const& buf, size_t const& len) {
			auto&& parsedLen = http_parser_execute(&parser, &parser_settings, buf, len);
			if (parsedLen < len) {
				return parser.http_errno;	// http_errno_description((http_errno)parser.http_errno)
			}
			return 0;
		}
	};

}
