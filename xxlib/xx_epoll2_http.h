#pragma once
#include "xx_epoll2.h"
#include "xx_http.h"

namespace xx::Epoll {

	struct HttpPeer;
	using HttpPeer_r = Ref<HttpPeer>;

	struct HttpListener : TcpListener {
		virtual TcpPeer_u OnCreatePeer() override;
	};

	struct HttpPeer : TcpPeer {
		// http 数据接收器
		HttpReceiver receiver;
		// http 数据下发器
		HttpResponse response;

		virtual void OnReceiveHttp(HttpContext& request, HttpResponse& response) = 0;

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

		virtual void OnReceive() override {
			if (recv.len) {
				// fill data to http receiver
				if (auto r = receiver.Input(recv.buf, recv.len)) {
					response.Send404Body(http_errno_description((http_errno)r));
				}
				// foreach finished context & callback
				auto&& count = receiver.GetFinishedCtxsCount();
				while (count) {
					auto&& request = receiver.ctxs.front();

					response.output.clear();
					OnReceiveHttp(request, response);

					receiver.ctxs.pop_front();
					--count;
				}
			}
			recv.Clear();
		}
	};

	inline TcpPeer_u HttpListener::OnCreatePeer() {
		return xx::TryMakeU<HttpPeer>();
	}
}
