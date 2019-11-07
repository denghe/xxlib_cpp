#pragma once
#include "xx_epoll.h"
#include "xx_httpreceiver.h"

namespace xx {
	struct SimpleHttpServer : xx::Epoll::Instance {

		inline virtual void OnAccept(xx::Epoll::Peer_r pr, int const& listenIndex) override {
			xx::CoutN(threadId, " OnAccept: id = ", pr->id, ", fd = ", pr->sockFD, ", ip = ", pr->ip);
			pr->userData = new HttpReceiver();
		}
		inline virtual void OnDisconnect(xx::Epoll::Peer_r pr) override {
			xx::CoutN(threadId, " OnDisconnect: id = ", pr->id, ", ip = ", pr->ip);
			delete (HttpReceiver*)pr->userData;
		}

		inline virtual int OnReceive(xx::Epoll::Peer_r pr) override {
			auto&& hp = (HttpReceiver*)pr->userData;
			if (auto&& r = hp->Input((char*)pr->recv.buf, pr->recv.len)) {
				SendResponse_404Body(http_errno_description((http_errno)r));
				return -1;
			}
			pr->recv.Clear();

			peer = pr;
			auto&& count = hp->GetFinishedCtxsCount();
			while (count) {
				request = &hp->ctxs.front();

				request->ParseUrl();
				auto&& iter = handlers.find(request->path);
				if (iter == handlers.end()) {
					SendResponse_404Body("the page not found!");
				}
				else {
					if (iter->second()) {
						SendResponse_404Body("bad request!");
					}
				}

				hp->ctxs.pop_front();
				--count;
			}
			return 0;
		}


		// 网址 path : 处理函数 映射填充到此
		std::unordered_map<std::string, std::function<int()>> handlers;

		// 指向当前 http 请求的连接
		xx::Epoll::Peer_r peer;

		// 指向当前 http 请求的上下文
		HttpContext* request = nullptr;


		// 兼容 text json 下发格式的前缀
		inline static std::string prefixText =
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: text/plain;charset=utf-8\r\n"
			"Connection: close\r\n"
			"Content-Length: ";

		inline static std::string prefixHtml =
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: text/html;charset=utf-8\r\n"
			"Connection: close\r\n"
			"Content-Length: ";

		inline static std::string prefix404 =
			"HTTP/1.1 404 Not Found\r\n"
			"Content-Type: text/html;charset=utf-8\r\n"
			"Connection: close\r\n"
			"Content-Length: ";

		// 下发 html 数据基础函数
		inline int SendResponse_Core(std::string const& prefix, char const* const& buf, std::size_t const& len) noexcept {
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

		// 公用输出拼接容器
		std::string response;

		// 会多一次复制但是方便拼接的下发 html 函数
		template<typename...Args>
		inline int SendResponse(std::string const& prefix, Args const& ...args) noexcept {
			response.clear();
			xx::Append(response, args...);
			return SendResponse_Core(prefix, response.data(), response.size());
		}

		inline int SendResponse_HtmlBody(std::string const& body) {
			return SendResponse(prefix404, "<html><body>", body, "</body></html>");
		}

		inline int SendResponse_404Body(std::string const& body) {
			return SendResponse(prefixHtml, "<html><body>", body, "</body></html>");
		}

		// todo: more helper funcs
	};
}
