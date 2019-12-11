#pragma once
#include "xx_epoll2.hpp"
#include "xx_http.h"

namespace xx::Epoll {

	struct HttpPeer;
	using HttpPeer_r = Ref<HttpPeer>;

	struct HttpListener : TcpListener {
		virtual TcpPeer_u OnCreatePeer() override;

		virtual void OnAccept(TcpPeer_r const& peer) override;

		// 网址 path : 处理函数 映射填充到此
		std::unordered_map<std::string, std::function<void(xx::HttpContext & q, xx::HttpResponse & r)>> handlers;
	};

	struct HttpPeer : TcpPeer {
		// 内含 处理函数映射
		Ref<HttpListener> listener;
		// http 数据接收器
		HttpReceiver receiver;
		// http 数据下发器
		HttpResponse response;

		virtual void OnReceive() override {
			if (recv.len) {
				// fill data to http receiver
				if (auto r = receiver.Input(recv.buf, recv.len)) {
					response.Send404Body(http_errno_description((http_errno)r));
				}
				// 定位到处理函数映射
				else if (auto&& L = listener.Lock()) {
					auto&& handlers = L->handlers;

					// 类自杀检测器
					Ref<HttpPeer> alive(this);

					// foreach finished context & callback
					auto&& count = receiver.GetFinishedCtxsCount();
					while (count) {
						auto&& request = receiver.ctxs.front();

						response.output.clear();
						{
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
								iter->second(request, response);

								// 如果执行过程中类自杀了就退出
								if (!alive) return;
							}
						}
						receiver.ctxs.pop_front();
						--count;
					}
				}
			}
			recv.Clear();
		}

		virtual void Init() override {
			response.onSend = [this](std::string const& prefix, char const* const& buf, size_t const& len)->int {
				// calc buf max len
				auto cap = /* partial http header */prefix.size() + /* len */20 + /*\r\n\r\n*/4 + /* body */len + /* refs */8;

				// alloc memory
				auto data = (char*)::malloc(cap);
				size_t dataLen = 0;

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
				Send(xx::Buf(dataLen, data));
				return 0;
			};
		}
	};

	inline TcpPeer_u HttpListener::OnCreatePeer() {
		return xx::TryMakeU<HttpPeer>();
	}
	inline void HttpListener::OnAccept(TcpPeer_r const& peer) {
		peer.As<HttpPeer>()->listener = this;
	}
}
