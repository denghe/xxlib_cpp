#pragma once
#include "xx_uv.h"
#include "http_parser.h"

namespace xx {

	struct UvHttpPeer;
	using UvHttpPeer_s = std::shared_ptr<UvHttpPeer>;

	struct UvHttpListener : UvItem {
		uv_tcp_t* uvTcp = nullptr;
		sockaddr_in6 addr;

		std::function<UvHttpPeer_s(Uv & uv)> onCreatePeer;
		std::function<void(UvHttpPeer_s peer)> onAccept;
		virtual UvHttpPeer_s CreatePeer() noexcept;
		virtual void Accept(UvHttpPeer_s peer) noexcept;


		UvHttpListener(Uv& uv, std::string const& ip, int const& port, int const& backlog = 128);

		UvHttpListener(UvHttpListener const&) = delete;
		UvHttpListener& operator=(UvHttpListener const&) = delete;
		~UvHttpListener() { this->Dispose(-1); }

		inline virtual bool Disposed() const noexcept override {
			return !uvTcp;
		}

		inline virtual bool Dispose(int const& flag = 1) noexcept override {
			if (!uvTcp) return false;
			Uv::HandleCloseAndFree(uvTcp);
			return true;
		}
	};


	// http 服务 peer 基类
	struct UvHttpPeer : UvItem {

		uv_tcp_t* uvTcp = nullptr;
		inline void TcpInit() {
			uvTcp = Uv::Alloc<uv_tcp_t>(this);
			if (!uvTcp) throw - 1;
			if (int r = uv_tcp_init(&uv.uvLoop, uvTcp)) {
				uvTcp = nullptr;
				throw r;
			}
		}

		inline virtual void Disconnect() noexcept {
			if (onDisconnect) {
				onDisconnect();
			}
		}

		inline virtual bool Dispose(int const& flag = 1) noexcept override {
			if (!uvTcp) return false;
			Uv::HandleCloseAndFree(uvTcp);
			auto holder = shared_from_this();
			if (flag == 1) {
				Disconnect();
			}
			onReceiveHttp = nullptr;
			onError = nullptr;
			return true;
		}

		~UvHttpPeer() { this->Dispose(-1); }

		inline virtual bool Disposed() const noexcept override {
			return !uvTcp;
		}

		// called by dialer or listener
		inline int ReadStart() noexcept {
			if (!uvTcp) return -1;
			return uv_read_start((uv_stream_t*)uvTcp, Uv::AllocCB, [](uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
				auto self = Uv::GetSelf<UvHttpPeer>(stream);
				auto holder = self->shared_from_this();	// hold for callback Dispose
				if (nread > 0) {
					auto&& parsedLen = (ssize_t)http_parser_execute(&self->parser, &self->parser_settings, buf->base, nread);
					if (parsedLen < nread) {
						if (self->onError) {
							self->onError(self->parser.http_errno, http_errno_description((http_errno)self->parser.http_errno));
						}
						self->Dispose();
					}
					// todo: 需要探测如果一次传入多份 http 完整信息会怎样, 是否触发多次回调。如果中间一次回调执行了 Dispose 会怎样？是否能终止解析并层层退出
				}
				if (buf) ::free(buf->base);
				if (nread < 0) {
					self->Dispose();
				}
				});
		}

		// ip:port
		std::string ip;

		// 成功接收完一段信息时的回调.
		std::function<int()> onReceiveHttp = []() noexcept { return 0; };

		// 接收出错回调. 接着会发生 Release
		std::function<void(uint32_t errorNumber, char const* errorMessage)> onError;

		// Dispose 时会触发
		std::function<void()> onDisconnect;


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



		// 返回文本 / json 的通用 http 头
		inline static const std::string responseHeader_Json =
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: text/plain;charset=utf-8\r\n"		// text/json
			"Content-Length: "
			;

		// 默认为解析 请求包. http_parser_type = HTTP_REQUEST, HTTP_RESPONSE, HTTP_BOTH
		UvHttpPeer(Uv& uv, http_parser_type parseType = HTTP_BOTH) : UvItem(uv) {
			TcpInit();

			parser.data = this;
			http_parser_init(&parser, parseType);

			http_parser_settings_init(&parser_settings);
			parser_settings.on_message_begin = [](http_parser* parser) noexcept {
				auto self = (UvHttpPeer*)parser->data;
				if (self->Disposed()) return -1;
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
				return self->onReceiveHttp();
			};
			parser_settings.on_chunk_header = [](http_parser* parser) noexcept { return 0; };
			parser_settings.on_chunk_complete = [](http_parser* parser) noexcept { return 0; };
		}

		// 快捷发回 json 内容响应
		inline int SendHttpResponse_Text(char const* const& bufPtr, std::size_t const& len) noexcept {
			// calc buf max len
			auto cap = sizeof(uv_write_t_ex) + /* http header */responseHeader_Json.size() + /* len */20 + /*\r\n\r\n*/4 + /* body */len;

			// alloc memory
			auto req = (uv_write_t_ex*)::malloc(sizeof(uv_write_t_ex) + cap);
			auto&& data = req->buf.base;
			auto&& dataLen = req->buf.len;

			// init
			data = (char*)req + sizeof(uv_write_t_ex);
			dataLen = 0;

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
			::memcpy(data + dataLen, bufPtr, len);
			dataLen += len;

			// send
			if (int r = uv_write(req, (uv_stream_t*)uvTcp, &req->buf, 1, [](uv_write_t* req, int status) { ::free(req); })) {
				Dispose();
				return r;
			}
			return 0;
		}

		inline int SendHttpResponse_Text(std::string const& json) noexcept {
			return SendHttpResponse_Text(json.data(), json.size());
		}

		// 会 urldecode 并 填充 path, queries, fragment. 需要手动调用
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

		inline std::string Dump() {
			std::string s;
			xx::Append(s
				, "url : ", url
				, "\nmethod : ", method
				, "\nbody : ", body
			);
			return s;
		}
	};

	inline UvHttpListener::UvHttpListener(Uv& uv, std::string const& ip, int const& port, int const& backlog)
		: UvItem(uv) {
		uvTcp = Uv::Alloc<uv_tcp_t>(this);
		if (!uvTcp) throw - 4;
		if (int r = uv_tcp_init(&uv.uvLoop, uvTcp)) {
			uvTcp = nullptr;
			throw r;
		}

		if (ip.find(':') == std::string::npos) {
			if (uv_ip4_addr(ip.c_str(), port, (sockaddr_in*)&addr)) throw - 1;
		}
		else {
			if (uv_ip6_addr(ip.c_str(), port, &addr)) throw - 2;
		}
		if (uv_tcp_bind(uvTcp, (sockaddr*)&addr, 0)) throw - 3;

		if (uv_listen((uv_stream_t*)uvTcp, backlog, [](uv_stream_t* server, int status) {
			if (status) return;
			auto&& self = Uv::GetSelf<UvHttpListener>(server);
			auto&& peer = self->CreatePeer();
			if (!peer) return;
			if (uv_accept(server, (uv_stream_t*)peer->uvTcp)) return;
			if (peer->ReadStart()) return;
			Uv::FillIP(peer->uvTcp, peer->ip);
			self->Accept(std::move(peer));
			})) throw - 4;
	};

	inline UvHttpPeer_s UvHttpListener::CreatePeer() noexcept {
		return xx::Make<UvHttpPeer>(uv);
	}

	inline void UvHttpListener::Accept(UvHttpPeer_s p) noexcept {
		if (onAccept) {
			onAccept(p);
		}
	}
}
