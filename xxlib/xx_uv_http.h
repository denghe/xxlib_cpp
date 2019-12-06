#pragma once
#include "xx_uv.h"
#include "xx_http.h"

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

	struct UvHttpPeer : UvItem {

		uv_tcp_t* uvTcp = nullptr;

		// ip:port
		std::string ip;

		// 成功接收完一段信息时的回调.
		std::function<int(HttpContext & request, HttpResponse & response)> onReceiveHttp;
		inline virtual int OnReceiveHttp(HttpContext& request, HttpResponse& response) {
			if (onReceiveHttp) {
				return onReceiveHttp(request, response);
			}
			else {
				return response.Send404Body("unhandled");
			}
		}

		// 接收出错回调. 接着会发生 Release
		std::function<void(int errorNumber, char const* errorMessage)> onError;

		// Dispose 时会触发
		std::function<void()> onDisconnect;

		// http 数据接收器
		HttpReceiver receiver;

		// http 数据下发器
		HttpResponse response;

		UvHttpPeer(Uv& uv)
			: UvItem(uv) {
			uvTcp = Uv::Alloc<uv_tcp_t>(this);
			if (!uvTcp) throw - 1;
			if (int r = uv_tcp_init(&uv.uvLoop, uvTcp)) {
				uvTcp = nullptr;
				throw r;
			}

			response.onSend = [this](std::string const& prefix, char const* const& buf, size_t const& len)->int {
				// calc buf max len
				auto cap = sizeof(uv_write_t_ex) + /* partial http header */prefix.size() + /* len */20 + /*\r\n\r\n*/4 + /* body */len;

				// alloc memory
				auto req = (uv_write_t_ex*)::malloc(sizeof(uv_write_t_ex) + cap);
				auto&& data = req->buf.base;
				auto&& dataLen = req->buf.len;

				// init
				data = (char*)req + sizeof(uv_write_t_ex);
				dataLen = 0;

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
				if (int r = uv_write(req, (uv_stream_t*)uvTcp, &req->buf, 1, [](uv_write_t* req, int status) { ::free(req); })) {
					Dispose();
					return r;
				}
				return 0;
			};
		}

		inline virtual void Disconnect() noexcept {
			if (onDisconnect) {
				onDisconnect();
			}
		}

		inline virtual bool Dispose(int const& flag = 1) noexcept override {
			if (!uvTcp) return false;
			Uv::HandleCloseAndFree(uvTcp);
			if (flag == -1) return true;
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
					// fill data to http receiver
					if (auto r = self->receiver.Input(buf->base, nread)) {
						if (self->onError) {
							self->onError(r, http_errno_description((http_errno)r));
						}
						else {
							self->response.Send404Body(http_errno_description((http_errno)r));
						}
					}
					// foreach finished context & callback
					auto&& count = self->receiver.GetFinishedCtxsCount();
					while (count) {
						auto&& request = self->receiver.ctxs.front();

						self->response.output.clear();
						self->OnReceiveHttp(request, self->response);

						self->receiver.ctxs.pop_front();
						--count;
					}
				}
				if (buf) ::free(buf->base);
				if (nread < 0) {
					self->Dispose();
				}
			});
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
