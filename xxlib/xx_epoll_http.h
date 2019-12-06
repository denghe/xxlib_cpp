#pragma once
#include "xx_epoll.h"
#include "xx_http.h"

namespace xx {
	struct SimpleHttpServer : xx::Epoll::Instance {

		// http 数据下发器
		HttpResponse response;

		// 需要绑处理函数或者覆盖虚函数
		std::function<int(HttpContext & request, HttpResponse & response)> onReceiveHttp;
		inline virtual int OnReceiveHttp(HttpContext& request, HttpResponse& response) {
			if (onReceiveHttp) {
				return onReceiveHttp(request, response);
			}
			else {
				return response.Send404Body("unhandled");
			}
		}

		inline virtual int OnReceive(xx::Epoll::Peer_r pr) override {
			// 得到接收器
			auto&& receiver = (HttpReceiver*)pr->userData;

			// 重设 response 输出函数，关联到当前 pr
			response.onSend = [pr](std::string const& prefix, char const* const& buf, size_t const& len)->int {
				if (!pr) return -1;

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
				return pr->Send(xx::Buf(dataLen, data));
			};

			if (auto&& r = receiver->Input((char*)pr->recv.buf, pr->recv.len)) {
				response.Send404Body(http_errno_description((http_errno)r));
				return -1;
			}
			pr->recv.Clear();


			auto&& count = receiver->GetFinishedCtxsCount();
			while (count) {
				auto&& request = receiver->ctxs.front();

				OnReceiveHttp(request, response);

				receiver->ctxs.pop_front();
				--count;
			}
			return 0;
		}

		inline virtual void OnAccept(xx::Epoll::Peer_r pr, int const& listenIndex) override {
			// 创建接收器
			pr->userData = new HttpReceiver();
		}

		inline virtual void OnDisconnect(xx::Epoll::Peer_r pr) override {
			// 回收接收器
			delete (HttpReceiver*)pr->userData;
		}
	};
}
