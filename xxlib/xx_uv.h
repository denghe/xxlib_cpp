#pragma once
#include "uv.h"
#include "xx_bbuffer.h"
#include "ikcp.h"

namespace xx {

	struct Uv {
		uv_loop_t uvLoop;
		BBuffer recvBB;						// shared deserialization for package receive. direct replace buf when using
		BBuffer sendBB;						// shared serialization for package send
		std::array<char, 65536> recvBuf;	// shared receive buf for kcp

		Uv() {
			if (int r = uv_loop_init(&uvLoop)) throw r;
		}
		Uv(Uv const&) = delete;
		Uv& operator=(Uv const&) = delete;

		~Uv() {
			recvBB.Reset();					// clear replaced buf.
			int r = uv_run(&uvLoop, UV_RUN_DEFAULT);
			xx::Cout("~UvLooop() uv_run return ", r);
			r = uv_loop_close(&uvLoop);
			xx::Cout(", uv_loop_close return ", r, "\n");
		}

		inline int Run(uv_run_mode const& mode = UV_RUN_DEFAULT) noexcept {
			return uv_run(&uvLoop, mode);
		}

		inline void Stop() {
			uv_stop(&uvLoop);
		}

		template<typename T>
		static T* Alloc(void* const& ud) noexcept {
			auto p = (void**)::malloc(sizeof(void*) + sizeof(T));
			if (!p) return nullptr;
			p[0] = ud;
			return (T*)&p[1];
		}
		inline static void Free(void* const& p) noexcept {
			::free((void**)p - 1);
		}
		template<typename T>
		static T* GetSelf(void* const& p) noexcept {
			return (T*)*((void**)p - 1);
		}
		template<typename T>
		static void HandleCloseAndFree(T*& tar) noexcept {
			if (!tar) return;
			auto h = (uv_handle_t*)tar;
			tar = nullptr;
			assert(!uv_is_closing(h));
			uv_close(h, [](uv_handle_t* handle) {
				Uv::Free(handle);
			});
		}
		inline static void AllocCB(uv_handle_t* h, size_t suggested_size, uv_buf_t* buf) noexcept {
			buf->base = (char*)::malloc(suggested_size);
			buf->len = decltype(uv_buf_t::len)(suggested_size);
		}
	};

	struct UvItem : std::enable_shared_from_this<UvItem> {
		Uv& uv;
		UvItem(Uv& uv) : uv(uv) {}
		virtual ~UvItem() {}
		// must be call Dispose() at all inherit class if override following funcs
		/*
			~TTTTTTTT() { if (!this->Disposed()) this->Dispose(); }
		*/
		virtual bool Disposed() const noexcept = 0;
		virtual void Dispose(int const& flag) noexcept = 0;
	};

	struct UvAsync : UvItem {
		uv_async_t* uvAsync = nullptr;
		std::mutex mtx;
		std::deque<std::function<void()>> actions;
		std::function<void()> action;	// for pop store

		UvAsync(Uv& uv)
			: UvItem(uv) {
			uvAsync = Uv::Alloc<uv_async_t>(this);
			if (!uvAsync) throw - 1;
			if (int r = uv_async_init(&uv.uvLoop, uvAsync, [](uv_async_t* handle) {
				Uv::GetSelf<UvAsync>(handle)->Execute();
			})) {
				uvAsync = nullptr;
				throw r;
			}
		}
		UvAsync(UvAsync const&) = delete;
		UvAsync& operator=(UvAsync const&) = delete;
		~UvAsync() { if (!this->Disposed()) this->Dispose(); }

		inline virtual bool Disposed() const noexcept override {
			return !uvAsync;
		}
		inline virtual void Dispose(int const& flag = 0) noexcept override {
			actions.clear();
			Uv::HandleCloseAndFree(uvAsync);
		}
		inline int Dispatch(std::function<void()>&& action) noexcept {
			if (!uvAsync) return -1;
			{
				std::scoped_lock<std::mutex> g(mtx);
				actions.push_back(std::move(action));
			}
			return uv_async_send(uvAsync);
		}

	protected:
		inline void Execute() noexcept {
			{
				std::scoped_lock<std::mutex> g(mtx);
				action = std::move(actions.front());
				actions.pop_front();
			}
			action();
		}
	};
	using UvAsync_s = std::shared_ptr<UvAsync>;
	using UvAsync_w = std::weak_ptr<UvAsync>;

	struct UvTimer : UvItem {
		uv_timer_t* uvTimer = nullptr;
		uint64_t timeoutMS = 0;
		uint64_t repeatIntervalMS = 0;
		std::function<void()> OnFire;

		UvTimer(Uv& uv)
			: UvItem(uv) {
			uvTimer = Uv::Alloc<uv_timer_t>(this);
			if (!uvTimer) throw - 1;
			if (int r = uv_timer_init(&uv.uvLoop, uvTimer)) {
				uvTimer = nullptr;
				throw r;
			}
		}
		UvTimer(UvTimer const&) = delete;
		UvTimer& operator=(UvTimer const&) = delete;
		~UvTimer() { if (!this->Disposed()) this->Dispose(); }

		inline virtual bool Disposed() const noexcept override {
			return !uvTimer;
		}
		inline virtual void Dispose(int const& flag = 0) noexcept override {
			Uv::HandleCloseAndFree(uvTimer);
			OnFire = nullptr;					// maybe cause free memory
		}

		inline int Start(uint64_t const& timeoutMS, uint64_t const& repeatIntervalMS, std::function<void()>&& onFire = nullptr) noexcept {
			if (!uvTimer) return -1;
			this->timeoutMS = timeoutMS;
			this->repeatIntervalMS = repeatIntervalMS;
			this->OnFire = std::move(onFire);
			return uv_timer_start(uvTimer, Fire, timeoutMS, repeatIntervalMS);
		}
		inline int Restart() noexcept {
			if (!uvTimer) return -1;
			return uv_timer_start(uvTimer, Fire, timeoutMS, repeatIntervalMS);
		}
		inline int Stop() noexcept {
			if (!uvTimer) return -1;
			return uv_timer_stop(uvTimer);
		}
		inline int Again() noexcept {
			if (!uvTimer) return -1;
			return uv_timer_again(uvTimer);
		}

	protected:
		inline static void Fire(uv_timer_t* t) {
			auto self = Uv::GetSelf<UvTimer>(t);
			if (self->OnFire) {
				self->OnFire();
			}
		}
	};
	using UvTimer_s = std::shared_ptr<UvTimer>;
	using UvTimer_w = std::weak_ptr<UvTimer>;

	struct UvResolver;
	using UvResolver_s = std::shared_ptr<UvResolver>;
	using UvResolver_w = std::weak_ptr<UvResolver>;
	struct uv_getaddrinfo_t_ex {
		uv_getaddrinfo_t req;
		UvResolver_w resolver_w;
	};

	struct UvResolver : UvItem {
		uv_getaddrinfo_t_ex* req = nullptr;
		UvTimer_s timeouter;
		std::vector<std::string> ips;
		std::function<void()> OnFinish;
#ifdef __IPHONE_OS_VERSION_MIN_REQUIRED
		addrinfo hints;
#endif

		UvResolver(Uv& uv) noexcept
			: UvItem(uv) {
			timeouter = xx::Make<UvTimer>(uv);
#ifdef __IPHONE_OS_VERSION_MIN_REQUIRED
			hints.ai_family = PF_UNSPEC;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_protocol = 0;// IPPROTO_TCP;
			hints.ai_flags = AI_DEFAULT;
#endif
		}

		UvResolver(UvResolver const&) = delete;
		UvResolver& operator=(UvResolver const&) = delete;
		~UvResolver() { if (!this->Disposed()) this->Dispose(); }

		inline virtual bool Disposed() const noexcept override {
			return !timeouter;
		}
		inline virtual void Dispose(int const& flag = 0) noexcept override {
			if (timeouter) {
				Cancel();
				OnFinish = nullptr;
				timeouter->Dispose();
			}
		}

		inline void Cancel() {
			if (!timeouter) return;
			ips.clear();
			if (req) {
				uv_cancel((uv_req_t*)req);
				req = nullptr;
			}
		}

		inline int Resolve(std::string const& domainName, uint64_t const& timeoutMS = 0) noexcept {
			if (!timeouter) return -1;
			Cancel();
			if (timeoutMS) {
				if (int r = timeouter->Start(timeoutMS, 0, [self_w = xx::AsWeak<UvResolver>(shared_from_this())]{
					if (auto self = self_w.lock()) {
						self->Dispose();
					}
					})) return r;
			}
			auto req = std::make_unique<uv_getaddrinfo_t_ex>();
			req->resolver_w = xx::As<UvResolver>(shared_from_this());
			if (int r = uv_getaddrinfo((uv_loop_t*)&uv.uvLoop, (uv_getaddrinfo_t*)&req->req, [](uv_getaddrinfo_t* req_, int status, struct addrinfo* ai) {
				auto req = std::unique_ptr<uv_getaddrinfo_t_ex>(container_of(req_, uv_getaddrinfo_t_ex, req));
				auto resolver = req->resolver_w.lock();
				if (!resolver) return;
				if (status) return;													// error or -4081 canceled
				assert(ai);

				auto& ips = resolver->ips;
				std::string s;
				do {
					s.resize(64);
					if (ai->ai_addr->sa_family == AF_INET6) {
						uv_ip6_name((sockaddr_in6*)ai->ai_addr, s.data(), s.size());
					}
					else {
						uv_ip4_name((sockaddr_in*)ai->ai_addr, s.data(), s.size());
					}
					s.resize(strlen(s.data()));

					if (std::find(ips.begin(), ips.end(), s) == ips.end()) {
						ips.push_back(std::move(s));								// known issue: ios || android will receive duplicate result
					}
					ai = ai->ai_next;
				} while (ai);
				uv_freeaddrinfo(ai);

				resolver->timeouter.reset();
				if (resolver->OnFinish) {
					resolver->OnFinish();
				}

			}, domainName.c_str(), nullptr,
#ifdef __IPHONE_OS_VERSION_MIN_REQUIRED
			(const addrinfo*)hints
#else
				nullptr
#endif
				)) return r;
			this->req = req.release();
			return 0;
		}
	};

	struct UvTcp : UvItem {
		uv_tcp_t* uvTcp = nullptr;

		UvTcp(Uv& uv)
			: UvItem(uv) {
			uvTcp = Uv::Alloc<uv_tcp_t>(this);
			if (!uvTcp) throw - 1;
			if (int r = uv_tcp_init(&uv.uvLoop, uvTcp)) {
				uvTcp = nullptr;
				throw r;
			}
		}
		~UvTcp() { if (!this->Disposed()) this->Dispose(); }

		inline virtual bool Disposed() const noexcept override {
			return !uvTcp;
		}
		inline virtual void Dispose(int const& flag = 0) noexcept override {
			Uv::HandleCloseAndFree(uvTcp);
		}
	};

	struct uv_write_t_ex : uv_write_t {
		uv_buf_t buf;
	};

	struct UvTcpBasePeer : UvTcp {
		Buffer buf;
		std::function<void()> OnDisconnect;
		inline virtual void Disconnect() noexcept { if (OnDisconnect) OnDisconnect(); }

		using UvTcp::UvTcp;
		UvTcpBasePeer(UvTcpBasePeer const&) = delete;
		UvTcpBasePeer& operator=(UvTcpBasePeer const&) = delete;
		~UvTcpBasePeer() { if (!this->Disposed()) this->Dispose(); }

		inline virtual void Dispose(int const& flag = 0) noexcept override {
			if (!uvTcp) return;
			this->UvTcp::Dispose(flag);
			if (flag) {
				auto holder = shared_from_this();
				Disconnect();		// maybe unhold memory here
				OnDisconnect = nullptr;
			}
		}

		// will be memcpy
		inline int Send(uint8_t const* const& buf, ssize_t const& dataLen) noexcept {
			if (!uvTcp) return -1;
			auto req = (uv_write_t_ex*)::malloc(sizeof(uv_write_t_ex) + dataLen);
			memcpy(req + 1, buf, dataLen);
			req->buf.base = (char*)(req + 1);
			req->buf.len = decltype(uv_buf_t::len)(dataLen);
			return SendReq(req);
		}

		// called by dialer or listener
		inline int ReadStart() noexcept {
			if (!uvTcp) return -1;
			return uv_read_start((uv_stream_t*)uvTcp, Uv::AllocCB, [](uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
				auto self = Uv::GetSelf<UvTcpBasePeer>(stream);
				if (nread > 0) {
					nread = self->Unpack((uint8_t*)buf->base, (uint32_t)nread);
				}
				if (buf) ::free(buf->base);
				if (nread < 0) {
					self->Dispose(1);	// call Disconnect
				}
			});
		}

	protected:
		// 4 byte len header. can override for write custom header format
		virtual int Unpack(uint8_t* const& recvBuf, uint32_t const& recvLen) noexcept {
			buf.AddRange(recvBuf, recvLen);
			uint32_t offset = 0;
			while (offset + 4 <= buf.len) {							// ensure header len( 4 bytes )
				auto len = buf[offset + 0] + (buf[offset + 1] << 8) + (buf[offset + 2] << 16) + (buf[offset + 3] << 24);
				if (len <= 0 /* || len > maxLimit */) return -1;	// invalid length
				if (offset + 4 + len > buf.len) break;				// not enough data

				offset += 4;
				if (int r = HandlePack(buf.buf + offset, len)) return r;
				offset += len;
			}
			buf.RemoveFront(offset);
			return 0;
		}

		// handle pack's content( except header )
		inline virtual int HandlePack(uint8_t* const& recvBuf, uint32_t const& recvLen) noexcept { return 0; };

		// launch a send request
		inline int SendReq(uv_write_t_ex* const& req) noexcept {
			if (!uvTcp) return -1;
			// todo: check send queue len ? protect?  uv_stream_get_write_queue_size((uv_stream_t*)uvTcp);
			int r = uv_write(req, (uv_stream_t*)uvTcp, &req->buf, 1, [](uv_write_t *req, int status) {
				::free(req);
			});
			if (r) Dispose(0);	// do not call Disconnect
			return r;
		}

		// fast mode. req + data 2N1, reduce malloc times.
		// reqbuf = uv_write_t_ex space + len space + data, len = data's len
		inline int SendReqAndData(uint8_t* const& reqbuf, uint32_t const& len) {
			reqbuf[sizeof(uv_write_t_ex) + 0] = uint8_t(len);		// fill package len
			reqbuf[sizeof(uv_write_t_ex) + 1] = uint8_t(len >> 8);
			reqbuf[sizeof(uv_write_t_ex) + 2] = uint8_t(len >> 16);
			reqbuf[sizeof(uv_write_t_ex) + 3] = uint8_t(len >> 24);

			auto req = (uv_write_t_ex*)reqbuf;						// fill req args
			req->buf.base = (char*)(req + 1);
			req->buf.len = decltype(uv_buf_t::len)(len + 4);
			return SendReq(req);
		}
	};

	// pack struct: [tar,] serial, data
	struct UvTcpPeer : UvTcpBasePeer {
		std::unordered_map<int, std::pair<std::function<int(Object_s&& msg)>, UvTimer_s>> callbacks;
		int serial = 0;
		std::function<int(Object_s&& msg)> OnReceivePush;
		inline virtual int ReceivePush(Object_s&& msg) noexcept { return OnReceivePush ? OnReceivePush(std::move(msg)) : 0; };
		std::function<int(int const& serial, Object_s&& msg)> OnReceiveRequest;
		inline virtual int ReceiveRequest(int const& serial, Object_s&& msg) noexcept { return OnReceiveRequest ? OnReceiveRequest(serial, std::move(msg)) : 0; };

		using UvTcpBasePeer::UvTcpBasePeer;
		UvTcpPeer(UvTcpPeer const&) = delete;
		UvTcpPeer& operator=(UvTcpPeer const&) = delete;
		~UvTcpPeer() { if (!this->Disposed()) this->Dispose(); }

		inline int SendPush(Object_s const& data) {
			return SendPackage(data);
		}
		inline int SendResponse(int32_t const& serial, Object_s const& data, int const& tar = 0) {
			return SendPackage(data, serial);
		}
		inline int SendRequest(Object_s const& msg, std::function<int(Object_s&& msg)>&& cb, uint64_t const& timeoutMS = 0) {
			return SendRequest(msg, 0, std::move(cb), timeoutMS);
		}

		inline virtual void Dispose(int const& flag = 1) noexcept override {
			if (!uvTcp) return;
			for (decltype(auto) kv : callbacks) {
				kv.second.first(nullptr);
			}
			callbacks.clear();
			this->UvTcp::Dispose(flag);
			if (flag) {
				auto holder = shared_from_this();
				Disconnect();		// maybe unhold memory here
				OnDisconnect = nullptr;
				OnReceivePush = nullptr;
				OnReceiveRequest = nullptr;
			}
		}

	protected:
		virtual int HandlePack(uint8_t* const& recvBuf, uint32_t const& recvLen) noexcept override {
			auto& recvBB = uv.recvBB;
			recvBB.Reset((uint8_t*)recvBuf, recvLen);

			int serial = 0;
			if (int r = recvBB.Read(serial)) return r;
			Object_s msg;
			if (int r = recvBB.ReadRoot(msg)) return r;

			if (serial == 0) {
				return ReceivePush(std::move(msg));
			}
			else if (serial < 0) {
				return ReceiveRequest(-serial, std::move(msg));
			}
			else {
				auto iter = callbacks.find(serial);
				if (iter == callbacks.end()) return 0;
				int r = iter->second.first(std::move(msg));
				callbacks.erase(iter);
				return r;
			}
		}

		// serial == 0: push    > 0: response    < 0: request
		inline int SendPackage(Object_s const& data, int32_t const& serial = 0, int const& tar = 0) {
			if (!uvTcp) return -1;
			auto& sendBB = uv.sendBB;
			static_assert(sizeof(uv_write_t_ex) + 4 <= 1024);
			sendBB.Reserve(1024);
			sendBB.len = sizeof(uv_write_t_ex) + 4;		// skip uv_write_t_ex + header space
			if (tar) sendBB.WriteFixed(tar);			// router package
			sendBB.Write(serial);
			sendBB.WriteRoot(data);

			auto buf = sendBB.buf;						// cut buf memory for send
			auto len = sendBB.len - sizeof(uv_write_t_ex) - 4;
			sendBB.buf = nullptr;
			sendBB.len = 0;
			sendBB.cap = 0;

			return SendReqAndData(buf, (uint32_t)len);
		}

		inline int SendRequest(Object_s const& data, int const& tar, std::function<int(Object_s&& msg)>&& cb, uint64_t const& timeoutMS = 0) {
			if (!uvTcp) return -1;
			std::pair<std::function<int(Object_s&& msg)>, UvTimer_s> v;
			++serial;
			if (timeoutMS) {
				v.second = xx::TryMake<UvTimer>(uv);
				if (!v.second) return -1;
				if (int r = v.second->Start(timeoutMS, 0, [this, serial = this->serial]() {
					TimeoutCallback(serial);
				})) return r;
			}
			if (int r = SendPackage(data, -serial, tar)) return r;
			v.first = std::move(cb);
			callbacks[serial] = std::move(v);
			return 0;
		}

		inline void TimeoutCallback(int const& serial) {
			auto iter = callbacks.find(serial);
			if (iter == callbacks.end()) return;
			iter->second.first(nullptr);
			callbacks.erase(iter);
		}
	};
	using UvTcpPeer_s = std::shared_ptr<UvTcpPeer>;
	using UvTcpPeer_w = std::weak_ptr<UvTcpPeer>;

	template<typename PeerType = UvTcpPeer>
	struct UvTcpListener : UvTcp {
		sockaddr_in6 addr;
		std::function<std::shared_ptr<PeerType>(Uv& uv)> OnCreatePeer;
		inline virtual std::shared_ptr<PeerType> CreatePeer() noexcept { return OnCreatePeer ? OnCreatePeer(uv) : xx::TryMake<PeerType>(uv); }
		std::function<void(std::shared_ptr<PeerType>& peer)> OnAccept;
		inline virtual void Accept(std::shared_ptr<PeerType>& peer) noexcept { if (OnAccept) OnAccept(peer); }

		UvTcpListener(Uv& uv, std::string const& ip, int const& port, int const& backlog = 128)
			: UvTcp(uv) {
			if (ip.find(':') == std::string::npos) {
				if (uv_ip4_addr(ip.c_str(), port, (sockaddr_in*)&addr)) throw - 1;
			}
			else {
				if (uv_ip6_addr(ip.c_str(), port, &addr)) throw - 2;
			}
			if (uv_tcp_bind(uvTcp, (sockaddr*)&addr, 0)) throw - 3;

			if (uv_listen((uv_stream_t*)uvTcp, backlog, [](uv_stream_t* server, int status) {
				if (status) return;
				auto self = Uv::GetSelf<UvTcpListener<PeerType>>(server);
				auto peer = self->CreatePeer();
				if (!peer) return;
				if (uv_accept(server, (uv_stream_t*)peer->uvTcp)) return;
				if (peer->ReadStart()) return;
				self->Accept(peer);
			})) throw - 4;
		};
		UvTcpListener(UvTcpListener const&) = delete;
		UvTcpListener& operator=(UvTcpListener const&) = delete;
	};

	template<typename PeerType = UvTcpPeer>
	struct UvTcpDialer;

	template<typename PeerType>
	struct uv_connect_t_ex {
		uv_connect_t req;
		std::shared_ptr<PeerType> peer;
		std::weak_ptr<UvTcpDialer<PeerType>> dialer_w;
		int serial;
		int batchNumber;
		~uv_connect_t_ex();
	};

	template<typename PeerType>
	struct UvTcpDialer : UvItem {
		using ThisType = UvTcpDialer<PeerType>;
		using ReqType = uv_connect_t_ex<PeerType>;
		int serial = 0;
		std::unordered_map<int, ReqType*> reqs;
		int batchNumber = 0;
		UvTimer_s timeouter;
		std::shared_ptr<PeerType> peer;
		std::function<std::shared_ptr<PeerType>(Uv& uv)> OnCreatePeer;
		inline virtual std::shared_ptr<PeerType> CreatePeer() noexcept { return OnCreatePeer ? OnCreatePeer(uv) : xx::TryMake<PeerType>(uv); }
		std::function<void()> OnConnect;
		inline virtual void Connect() noexcept { if (OnConnect) OnConnect(); }

		UvTcpDialer(Uv& uv)
			: UvItem(uv) {
			timeouter = xx::Make<UvTimer>(uv);
		}
		UvTcpDialer(UvTcpDialer const&) = delete;
		UvTcpDialer& operator=(UvTcpDialer const&) = delete;
		~UvTcpDialer() { if (!this->Disposed()) this->Dispose(); }

		inline virtual bool Disposed() const noexcept override {
			return !timeouter;
		}
		inline virtual void Dispose(int const& flag = 0) noexcept override {
			if (timeouter) {
				Cancel();
				timeouter->Dispose();
			}
		}

		// 0: free    1: dialing    2: connected		3: disposed
		inline int State() const noexcept {
			if (!timeouter) return 3;
			if (peer && !peer->Disposed()) return 2;
			if (reqs.size()) return 1;
			return 0;
		}

		inline int Dial(std::string const& ip, int const& port, uint64_t const& timeoutMS = 0, bool cleanup = true) noexcept {
			if (!timeouter) return -1;
			if (cleanup) {
				Cancel();
			}

			sockaddr_in6 addr;
			if (ip.find(':') == std::string::npos) {								// ipv4
				if (int r = uv_ip4_addr(ip.c_str(), port, (sockaddr_in*)&addr)) return r;
			}
			else {																	// ipv6
				if (int r = uv_ip6_addr(ip.c_str(), port, &addr)) return r;
			}

			if (int r = SetTimeout(timeoutMS)) return r;

			auto req = std::make_unique<ReqType>();
			req->peer = CreatePeer();
			req->dialer_w = xx::As<ThisType>(shared_from_this());
			req->serial = ++serial;
			req->batchNumber = batchNumber;

			if (uv_tcp_connect(&req->req, req->peer->uvTcp, (sockaddr*)&addr, [](uv_connect_t* conn, int status) {
				auto req = std::unique_ptr<ReqType>(container_of(conn, ReqType, req));
				auto client = req->dialer_w.lock();
				if (!client) return;
				if (status) return;													// error or -4081 canceled
				if (client->batchNumber > req->batchNumber) return;
				if (client->peer) return;											// only fastest connected peer can survival

				if (req->peer->ReadStart()) return;
				client->peer = std::move(req->peer);								// connect success
				client->timeouter.reset();
				client->Connect();
			})) return -3;

			reqs[serial] = req.release();
			return 0;
		}

		inline int Dial(std::vector<std::string> const& ips, int const& port, uint64_t const& timeoutMS = 0) noexcept {
			if (!timeouter) return -1;
			Cancel();
			if (int r = SetTimeout(timeoutMS)) return r;
			for (decltype(auto) ip : ips) {
				Dial(ip, port, 0, false);
			}
		}
		inline int Dial(std::vector<std::pair<std::string, int>> const& ipports, uint64_t const& timeoutMS = 0) noexcept {
			if (!timeouter) return -1;
			Cancel();
			if (int r = SetTimeout(timeoutMS)) return r;
			for (decltype(auto) ipport : ipports) {
				Dial(ipport.first, ipport.second, 0, false);
			}
		}

		inline void Cancel(bool resetPeer = true) noexcept {
			if (!timeouter) return;
			timeouter->Stop();
			if (resetPeer) {
				peer.reset();
			}
			for (decltype(auto) kv : reqs) {
				uv_cancel((uv_req_t*)kv.second);
			}
			reqs.clear();
			serial = 0;
			++batchNumber;
		}

	protected:
		inline int SetTimeout(uint64_t const& timeoutMS = 0) noexcept {
			if (!timeouter) return -1;
			int r = timeouter->Stop();
			if (!timeoutMS) return r;
			return timeouter->Start(timeoutMS, 0, [self_w = xx::AsWeak<UvTcpDialer>(shared_from_this())]{
				if (auto self = self_w.lock()) {
					self->Cancel(true);
					self->Connect();
				}
				});
		}
	};

	template<typename PeerType>
	inline uv_connect_t_ex<PeerType>::~uv_connect_t_ex() {
		if (auto client = dialer_w.lock()) {
			client->reqs.erase(serial);
		}
	}


	struct uv_udp_send_t_ex : uv_udp_send_t {
		uv_buf_t buf;
	};

	struct UvUdpBasePeer : UvItem {
		uv_udp_t* uvUdp = nullptr;
		sockaddr_in6 addr;
		Buffer buf;

		std::function<void()> OnDisconnect;
		inline virtual void Disconnect() noexcept { if (OnDisconnect) OnDisconnect(); }

		UvUdpBasePeer(Uv& uv, std::string const& ip, int const& port, bool const& isListener)
			: UvItem(uv) {
			if (ip.size()) {
				if (ip.find(':') == std::string::npos) {
					if (int r = uv_ip4_addr(ip.c_str(), port, (sockaddr_in*)&addr)) throw r;
				}
				else {
					if (int r = uv_ip6_addr(ip.c_str(), port, &addr)) throw r;
				}
			}
			uvUdp = Uv::Alloc<uv_udp_t>(this);
			if (!uvUdp) throw - 2;
			if (int r = uv_udp_init(&uv.uvLoop, uvUdp)) {
				uvUdp = nullptr;
				throw r;
			}
			xx::ScopeGuard sgUdp([this] { Uv::HandleCloseAndFree(uvUdp); });
			if (isListener) {
				if (int r = uv_udp_bind(uvUdp, (sockaddr*)&addr, UV_UDP_REUSEADDR)) throw r;
			}
			if (int r = uv_udp_recv_start(uvUdp, Uv::AllocCB, [](uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags) {
				if (nread > 0) {
					nread = Uv::GetSelf<UvUdpBasePeer>(handle)->Unpack((uint8_t*)buf->base, (uint32_t)nread, addr);
					assert(!nread);
				}
				if (buf) ::free(buf->base);
				if (nread < 0) {
					Uv::GetSelf<UvUdpBasePeer>(handle)->Dispose(true);
				}
			})) throw r;
			sgUdp.Cancel();
		}
		UvUdpBasePeer(UvUdpBasePeer const&) = delete;
		UvUdpBasePeer& operator=(UvUdpBasePeer const&) = delete;
		~UvUdpBasePeer() { if (!this->Disposed()) this->Dispose(); }

		inline virtual bool Disposed() const noexcept override {
			return !uvUdp;
		}
		inline virtual void Dispose(int const& flag = 0) noexcept override {
			if (!uvUdp) return;
			Uv::HandleCloseAndFree(uvUdp);
			if (flag) {
				Disconnect();		// maybe unhold memory here
				OnDisconnect = nullptr;
			}
		}

		// send target: addr or this->addr
		inline int Send(uint8_t const* const& buf, ssize_t const& dataLen, sockaddr const* const& addr = nullptr) noexcept {
			if (!uvUdp) return -1;
			auto req = (uv_udp_send_t_ex*)::malloc(sizeof(uv_udp_send_t_ex) + dataLen);
			memcpy(req + 1, buf, dataLen);
			req->buf.base = (char*)(req + 1);
			req->buf.len = decltype(uv_buf_t::len)(dataLen);
			return Send(req, addr);
		}

	protected:
		virtual int Unpack(uint8_t* const& recvBuf, uint32_t const& recvLen, sockaddr const* const& addr) noexcept {
			buf.AddRange(recvBuf, recvLen);
			uint32_t offset = 0;
			while (offset + 4 <= buf.len) {							// ensure header len( 4 bytes )
				auto len = buf[offset + 0] + (buf[offset + 1] << 8) + (buf[offset + 2] << 16) + (buf[offset + 3] << 24);
				if (len <= 0 /* || len > maxLimit */) return -1;	// invalid length
				if (offset + 4 + len > buf.len) break;				// not enough data

				offset += 4;
				if (int r = HandlePack(buf.buf + offset, len, addr)) return r;
				offset += len;
			}
			buf.RemoveFront(offset);
			return 0;
		}

		virtual int HandlePack(uint8_t* const& recvBuf, uint32_t const& recvLen, sockaddr const* const& addr) { return 0; };

		// reqbuf = uv_udp_send_t_ex space + len space + data
		// len = data's len
		inline int SendReqAndData(uint8_t* const& reqbuf, uint32_t const& len, sockaddr const* const& addr = nullptr) {
			reqbuf[sizeof(uv_udp_send_t_ex) + 0] = uint8_t(len);		// fill package len
			reqbuf[sizeof(uv_udp_send_t_ex) + 1] = uint8_t(len >> 8);
			reqbuf[sizeof(uv_udp_send_t_ex) + 2] = uint8_t(len >> 16);
			reqbuf[sizeof(uv_udp_send_t_ex) + 3] = uint8_t(len >> 24);

			auto req = (uv_udp_send_t_ex*)reqbuf;						// fill req args
			req->buf.base = (char*)(req + 1);
			req->buf.len = decltype(uv_buf_t::len)(len + 4);
			return Send(req, addr);
		}

		inline int Send(uv_udp_send_t_ex* const& req, sockaddr const* const& addr = nullptr) noexcept {
			if (!uvUdp) return -1;
			// todo: check send queue len ? protect?
			int r = uv_udp_send(req, uvUdp, &req->buf, 1, addr ? addr : (sockaddr*)&this->addr, [](uv_udp_send_t *req, int status) {
				::free(req);
			});
			if (r) Dispose();
			return r;
		}
	};

	struct UvUdpKcpPeer : UvItem {
		ikcpcb* kcp = nullptr;
		int serial = 0;
		std::unordered_map<int, std::pair<std::function<int(Object_s&& msg)>, UvTimer_s>> callbacks;
		uint64_t createMS = 0;				// fill after new
		uint64_t currentMS = 0;				// fill before Update by dialer / listener
		uint64_t nextUpdateMS = 0;			// for kcp update interval control. reduce cpu usage
		UvUdpBasePeer* owner;				// for Send
		Buffer buf;
		Guid g;
		sockaddr_in6 addr;					// for Send. fill by owner Unpack
		uint64_t receiveTimeoutMS = 0;		// if receiveTimeoutMS > 0 && Update current - lastReceiveMS > receiveTimeoutMS , will Dispose
		uint64_t lastReceiveMS = 0;			// refresh at Update when receive data

		std::function<void()> OnDisconnect;
		inline virtual void Disconnect() noexcept { if (OnDisconnect) OnDisconnect(); }
		// return !0 will Dispose
		std::function<int(Object_s&& msg)> OnReceivePush;
		inline virtual int ReceivePush(Object_s&& msg) noexcept { return OnReceivePush ? OnReceivePush(std::move(msg)) : 0; };
		std::function<int(int const& serial, Object_s&& msg)> OnReceiveRequest;
		inline virtual int ReceiveRequest(int const& serial, Object_s&& msg) noexcept { return OnReceiveRequest ? OnReceiveRequest(serial, std::move(msg)) : 0; };

		UvUdpKcpPeer(UvUdpBasePeer* const& owner, Guid const& g)
			: UvItem(owner->uv)
			, owner(owner)
			, g(g) {
			kcp = ikcp_create(g, this);
			if (!kcp) throw - 1;
			xx::ScopeGuard sgKcp([&] { ikcp_release(kcp); kcp = nullptr; });
			if (int r = ikcp_wndsize(kcp, 128, 128)) throw r;
			if (int r = ikcp_nodelay(kcp, 1, 10, 2, 1)) throw r;
			kcp->rx_minrto = 10;
			ikcp_setoutput(kcp, [](const char *inBuf, int len, ikcpcb *kcp, void *user)->int {
				auto self = ((UvUdpKcpPeer*)user);
				return self->owner->Send((uint8_t*)inBuf, len, (sockaddr*)&self->addr);
			});
			sgKcp.Cancel();
		}
		UvUdpKcpPeer(UvUdpKcpPeer const&) = delete;
		UvUdpKcpPeer& operator=(UvUdpKcpPeer const&) = delete;
		~UvUdpKcpPeer() { if (!this->Disposed()) this->Dispose(); }

		inline int SendPush(Object_s const& data) {
			return SendPackage(data);
		}
		inline int SendResponse(int32_t const& serial, Object_s const& data) {
			return SendPackage(data, serial);
		}
		inline int SendRequest(Object_s const& msg, std::function<int(Object_s&& msg)>&& cb, uint64_t const& timeoutMS = 0) {
			return SendRequest(msg, 0, std::move(cb), timeoutMS);
		}

		// send data immediately ( no wait for more data combine send )
		inline void Flush() {
			if (!kcp) return;
			ikcp_flush(kcp);
		}

		// set timeout check duration. 0 = disable check
		inline void SetReceiveTimeoutMS(uint64_t const& receiveTimeoutMS) {
			this->receiveTimeoutMS = receiveTimeoutMS;
		}

		// refresh lastReceiveMS when receive a valid message
		inline void ResetLastReceiveMS() {
			lastReceiveMS = currentMS;
		}

		template<typename T>
		T* GetOwner() {
			assert(dynamic_cast<T>(owner));
			return (T*)owner;
		}

		inline virtual bool Disposed() const noexcept override {
			return !kcp;
		}
		inline virtual void Dispose(int const& flag = 0) noexcept override {
			if (!kcp) return;
			ikcp_release(kcp);
			kcp = nullptr;
			for (decltype(auto) kv : callbacks) {
				kv.second.first(nullptr);
			}
			callbacks.clear();
			if (flag) {
				auto holder = shared_from_this();
				Disconnect();						// maybe unhold memory here
				OnDisconnect = nullptr;
				OnReceivePush = nullptr;
				OnReceiveRequest = nullptr;
			}
		}

		// called by udp kcp dialer or listener
		// put data to kcp when udp receive 
		inline int Input(uint8_t* const& recvBuf, uint32_t const& recvLen) noexcept {
			if (!kcp) return -1;
			return ikcp_input(kcp, (char*)recvBuf, recvLen);
		}

		// called by udp kcp dialer or listener
		// timer call this for recv data from kcp
		inline int Update(uint64_t const& nowMS) noexcept {
			currentMS = nowMS - createMS;
			if (receiveTimeoutMS) {
				if (receiveTimeoutMS + lastReceiveMS < currentMS) 
					return -1;	// receive timeout check
			}
			if (nextUpdateMS > currentMS) return 0;
			ikcp_update(kcp, (uint32_t)currentMS);
			nextUpdateMS = ikcp_check(kcp, (uint32_t)currentMS);
			do {
				int recvLen = ikcp_recv(kcp, uv.recvBuf.data(), (int)uv.recvBuf.size());
				if (recvLen <= 0) break;
				if (int r = Unpack((uint8_t*)uv.recvBuf.data(), recvLen)) return r;
			} while (true);
			return 0;
		}

	protected:
		// 4 bytes len header. can override for custom header format.
		inline virtual int Unpack(uint8_t* const& recvBuf, uint32_t const& recvLen) noexcept {
			buf.AddRange(recvBuf, recvLen);
			uint32_t offset = 0;
			while (offset + 4 <= buf.len) {							// ensure header len( 4 bytes )
				auto len = buf[offset + 0] + (buf[offset + 1] << 8) + (buf[offset + 2] << 16) + (buf[offset + 3] << 24);
				if (len <= 0 /* || len > maxLimit */) return -1;	// invalid length
				if (offset + 4 + len > buf.len) break;				// not enough data

				offset += 4;
				if (int r = HandlePack(buf.buf + offset, len)) return r;
				offset += len;
			}
			buf.RemoveFront(offset);
			return 0;
		}

		// unpack & dispatch
		inline virtual int HandlePack(uint8_t* const& recvBuf, uint32_t const& recvLen) noexcept {
			auto& recvBB = uv.recvBB;
			recvBB.Reset((uint8_t*)recvBuf, recvLen);

			int serial = 0;
			if (int r = recvBB.Read(serial)) return r;
			Object_s msg;
			if (int r = recvBB.ReadRoot(msg)) return r;

			if (serial == 0) {
				return ReceivePush(std::move(msg));
			}
			else if (serial < 0) {
				return ReceiveRequest(-serial, std::move(msg));
			}
			else {
				auto iter = callbacks.find(serial);
				if (iter == callbacks.end()) return 0;
				int r = iter->second.first(std::move(msg));
				callbacks.erase(iter);
				return r;
			}
		}

		// put send data into kcp. though ikcp_setoutput func send.
		inline int Send(uint8_t const* const& buf, ssize_t const& dataLen) noexcept {
			if (!kcp) return -1;
			return ikcp_send(kcp, (char*)buf, (int)dataLen);
		}

		// serial == 0: push    > 0: response    < 0: request
		inline int SendPackage(Object_s const& data, int32_t const& serial = 0, int const& tar = 0) {
			if (!kcp) return -1;
			auto& sendBB = uv.sendBB;
			sendBB.Resize(4);						// header len( 4 bytes )
			if (tar) sendBB.WriteFixed(tar);		// for router
			sendBB.Write(serial);
			sendBB.WriteRoot(data);
			auto buf = sendBB.buf;
			auto len = sendBB.len - 4;
			buf[0] = uint8_t(len);					// fill package len
			buf[1] = uint8_t(len >> 8);
			buf[2] = uint8_t(len >> 16);
			buf[3] = uint8_t(len >> 24);
			return Send(buf, sendBB.len);
		}

		inline int SendRequest(Object_s const& data, int const& tar, std::function<int(Object_s&& msg)>&& cb, uint64_t const& timeoutMS = 0) {
			if (!kcp) return -1;
			std::pair<std::function<int(Object_s&& msg)>, UvTimer_s> v;
			++serial;
			if (timeoutMS) {
				v.second = xx::TryMake<UvTimer>(uv);
				if (!v.second) return -1;
				if (int r = v.second->Start(timeoutMS, 0, [this, serial = this->serial]() {
					TimeoutCallback(serial);
				})) return r;
			}
			if (int r = SendPackage(data, -serial, tar)) return r;
			v.first = std::move(cb);
			callbacks[serial] = std::move(v);
			return 0;
		}

		inline void TimeoutCallback(int const& serial) {
			auto iter = callbacks.find(serial);
			if (iter == callbacks.end()) return;
			iter->second.first(nullptr);
			callbacks.erase(iter);
		}
	};
	using UvUdpKcpPeer_s = std::shared_ptr<UvUdpKcpPeer>;
	using UvUdpKcpPeer_w = std::weak_ptr<UvUdpKcpPeer>;

	template<typename PeerType>
	struct UvUdpBasePeerKcpEx : UvUdpBasePeer {
		UvTimer_s updater;
		int64_t currentMS = 0;
		std::chrono::steady_clock::time_point createTime = std::chrono::steady_clock::now();
		std::function<std::shared_ptr<PeerType>(UvUdpBasePeer* const& owner, Guid const& g)> OnCreatePeer;
		inline virtual std::shared_ptr<PeerType> CreatePeer(Guid const& g) noexcept { return OnCreatePeer ? OnCreatePeer(this, g) : xx::TryMake<PeerType>(this, g); }

		UvUdpBasePeerKcpEx(Uv& uv, std::string const& ip, int const& port, bool const& isListener)
			: UvUdpBasePeer(uv, ip, port, isListener) {
			xx::MakeTo(updater, uv);
		}
	};

	template<typename PeerType = UvUdpKcpPeer>
	struct UvUdpKcpListener : UvUdpBasePeerKcpEx<PeerType> {
		using PeerType_s = std::shared_ptr<PeerType>;
		using BaseType = UvUdpBasePeerKcpEx<PeerType>;
		std::unordered_map<xx::Guid, std::weak_ptr<PeerType>> peers;
		std::vector<xx::Guid> dels;
		std::function<void(std::shared_ptr<PeerType>& peer)> OnAccept;
		inline virtual void Accept(std::shared_ptr<PeerType>& peer) noexcept { if (OnAccept) OnAccept(peer); }

		UvUdpKcpListener(Uv& uv, std::string const& ip, int const& port)
			: BaseType(uv, ip, port, true) {
			if (int r = this->updater->Start(10, 16, [this] {
				this->currentMS = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - this->createTime).count();
				for (decltype(auto) kv : peers) {		// todo: timeout check?
					auto peer = kv.second.lock();
					if (peer && !peer->Disposed()) {
						if (peer->Update(this->currentMS)) {
							peer->Dispose(1);
							dels.push_back(kv.first);
						}
					}
					else {
						dels.push_back(kv.first);
					}
				}
				for (decltype(auto) g : dels) {
					peers.erase(g);
				}
				dels.clear();
			})) throw r;
		}
		UvUdpKcpListener(UvUdpKcpListener const&) = delete;
		UvUdpKcpListener& operator=(UvUdpKcpListener const&) = delete;
		~UvUdpKcpListener() { if (!this->Disposed()) this->Dispose(); }

		inline virtual void Dispose(int const& flag = 0) noexcept override {
			if (this->Disposed()) return;
			for (decltype(auto) kv : peers) {
				if (auto peer = kv.second.lock()) {
					peer->Dispose(flag);
				}
			}
			Uv::HandleCloseAndFree(this->uvUdp);
			if (flag) {
				this->Disconnect();		// maybe unhold memory here
				this->OnDisconnect = nullptr;
				OnAccept = nullptr;
			}
		}

	protected:
		virtual int Unpack(uint8_t* const& recvBuf, uint32_t const& recvLen, sockaddr const* const& addr) noexcept override {
			// header 至少有 36 字节长( Guid conv 的 kcp 头 )
			// 少于 36 的直接 echo 回去方便探测 ping 值( todo: 应对被伪造目标地址指向别处的请况 )
			if (recvLen < 36) {
				return this->Send(recvBuf, recvLen, addr);
			}
			Guid g(false);
			g.Fill(recvBuf);				// 前 16 字节转为 Guid
			auto iter = peers.find(g);		// 去字典中找. 没有就新建.
			PeerType_s p = iter == peers.end() ? nullptr : iter->second.lock();
			if (!p) {
				p = this->CreatePeer(g);
				if (!p) return 0;
				p->createMS = this->currentMS;
				peers[g] = p;
			}
			memcpy(&p->addr, addr, sizeof(sockaddr_in6));	// 更新 peer 的目标 ip 地址		// todo: ip changed 事件?? 严格模式? ip 变化就清除 peer
			if (iter == peers.end()) {
				Accept(p);
			}
			if (p->Disposed()) return 0;
			if (p->Input(recvBuf, recvLen)) {
				p->Dispose();
			}
			return 0;
		}
	};

	template<typename PeerType = UvUdpKcpPeer>
	struct UvUdpKcpDialer : UvUdpBasePeerKcpEx<PeerType> {
		using BaseType = UvUdpBasePeerKcpEx<PeerType>;
		using PeerType_s = std::shared_ptr<PeerType>;
		xx::Guid g;
		PeerType_s peer;		// Dial timeout will be nullptr
		std::function<void()> OnConnect;
		inline virtual void Connect() noexcept { if (OnConnect) OnConnect(); }
		UvTimer_s timer;		// 暂时用来延迟调用 Connect() 函数以避免 Disconnect 时 Dial 后 peer 被 Dispose 的尴尬

		UvUdpKcpDialer(Uv& uv)
			: BaseType(uv, "", 0, false) {
			if (int r = this->updater->Start(10, 10, [this] {
				this->currentMS = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - this->createTime).count();
				if (peer && !peer->Disposed()) {
					if (peer->Update(this->currentMS)) {
						peer->Dispose(1);
						peer.reset();
					}
				}
			})) throw r;
			xx::MakeTo(timer, uv);
		}
		UvUdpKcpDialer(UvUdpKcpDialer const&) = delete;
		UvUdpKcpDialer& operator=(UvUdpKcpDialer const&) = delete;
		~UvUdpKcpDialer() { if (!this->Disposed()) this->Dispose(); }

		inline virtual void Dispose(int const& flag = 0) noexcept override {
			if (this->Disposed()) return;
			if (peer) {
				peer->Dispose();
				peer.reset();
			}
			timer.reset();
			Uv::HandleCloseAndFree(this->uvUdp);
			if (flag) {
				this->Disconnect();		// maybe unhold memory here
				this->OnDisconnect = nullptr;
				OnConnect = nullptr;
			}
		}

		// todo: 模拟握手？ peer 创建之后发送握手包并等待返回？
		// todo: 批量拨号？
		// todo: 实现一个与 kcp 无关的握手流程？只发送 guid? 直接返回 guid? 使用一个 timer 来管理? 不停的发，直到收到？

		inline int Dial(std::string const& ip, int const& port) noexcept {
			if (int r = timer->Stop()) return r;
			return timer->Start(1, 0, [this, ip, port] {
				g.Gen();
				peer = this->CreatePeer(g);
				if (!peer) return -1;
				xx::ScopeGuard sgPeer([&] { peer.reset(); });

				if (ip.find(':') == std::string::npos) {								// ipv4
					if (int r = uv_ip4_addr(ip.c_str(), port, (sockaddr_in*)&peer->addr)) return r;
				}
				else {																	// ipv6
					if (int r = uv_ip6_addr(ip.c_str(), port, &peer->addr)) return r;
				}

				peer->createMS = this->currentMS;
				sgPeer.Cancel();
				Connect();
			});
		}

	protected:
		inline virtual int Unpack(uint8_t* const& recvBuf, uint32_t const& recvLen, sockaddr const* const& addr) noexcept override {
			if (!peer || peer->Disposed()) return 0;	// 没建立对端
			// todo: 校验 addr 是否与 Dial 时传递的一致
			if (recvLen < 36) return 0;					// header 至少有 36 字节长( Guid conv 的 kcp 头 )
			Guid g(false);
			g.Fill(recvBuf);							// 前 16 字节转为 Guid
			if (this->g != g) return 0;					// 有可能是延迟收到上个连接的残包
			if (peer->Input(recvBuf, recvLen)) {
				peer->Dispose();
			}
			return 0;
		}
	};
}
