#include "xx_uv.h"
#include "xx_dict.h"

namespace xx {
	struct KcpUv;

	struct KcpPeer;
	using KcpPeer_s = std::shared_ptr<KcpPeer>;
	using KcpPeer_w = std::weak_ptr<KcpPeer>;

	struct KcpListener;
	using KcpListener_s = std::shared_ptr<KcpListener>;
	using KcpListener_w = std::weak_ptr<KcpListener>;

	struct KcpUvUdpPeer : UvUdpBasePeer {
		using UvUdpBasePeer::UvUdpBasePeer;
		uint16_t port = 0;
		KcpUv& GetKcpUv() noexcept;
		~KcpUvUdpPeer();
		KcpListener* listener = nullptr;
		// todo: listener pointer for call listener's accept func or call dialer's accpet func
	};
	using KcpUvUdpPeer_s = std::shared_ptr<KcpUvUdpPeer>;
	using KcpUvUdpPeer_w = std::weak_ptr<KcpUvUdpPeer>;

	struct KcpPeer : UvItem {
		KcpUvUdpPeer_s udpPeer;								// bind to udp peer
		Guid guid;
		int Update(uint32_t const& currentMS) {
			// todo
			return 0;
		}
		KcpUv& GetKcpUv() noexcept;
		KcpPeer(Uv& uv, KcpUvUdpPeer_s& udpPeer) 
			: UvItem(uv)
			, udpPeer(std::move(udpPeer)) {
		}
		~KcpPeer();
	};

	struct KcpListener : UvItem {
		KcpUvUdpPeer_s udpPeer;
		// todo: accept
		KcpUv& GetKcpUv() noexcept;
		KcpListener(Uv& uv, std::string const& ip, int const& port);
		~KcpListener() {
			udpPeer->listener = nullptr;
		}
	};
	struct KcpDialer : UvItem {
		using UvItem::UvItem;
		KcpUv& GetKcpUv() noexcept;
		// todo: Dial create tmp KcpUvUdpPeer req, send guid & wait recv guid, when recv, callback Connect
		// todo: req container
	};

	struct KcpUv : Uv {
		Dict<uint16_t, KcpUvUdpPeer_w> udpPeers;			// search udp peer by create new listener
		Dict<Guid, KcpPeer_w> kcpPeers;						// for update
		UvTimer_s updater;
		std::chrono::steady_clock::time_point createTime = std::chrono::steady_clock::now();
		uint32_t currentMS = 0;	// why not int64_t: because kcp source code only support this type
		KcpUv() {
			xx::MakeTo(updater, *this, 10, 10, [this] {
				currentMS = (uint32_t)(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - this->createTime).count());
				for (auto&& iter = kcpPeers.begin(); iter != kcpPeers.end(); ++iter) {
					if (auto peer = (*iter).value.lock()) {
						if (!peer->Update(currentMS)) continue;
						peer->Dispose(1);
					}
					kcpPeers.RemoveAt(iter.i);				// remove !lock or Update failed
				}
			});
		}
	};

	inline KcpListener::KcpListener(Uv& uv, std::string const& ip, int const& port)
		: UvItem(uv) {
		auto&& ps = GetKcpUv().udpPeers;
		auto idx = ps.Find(port);
		if (idx > -1) {
			udpPeer = ps.ValueAt(idx).lock();
			if (udpPeer->listener) throw - 1;		// same port listener already exists?
		}
		else {
			xx::MakeTo(udpPeer, uv, ip, port, true);
			ps[port] = udpPeer;
		}
		udpPeer->listener = this;
		// todo
	}

	KcpUv& KcpUvUdpPeer::GetKcpUv() noexcept {
		return (KcpUv&)uv;
	}
	KcpUv& KcpPeer::GetKcpUv() noexcept {
		return (KcpUv&)uv;
	}
	KcpUv& KcpListener::GetKcpUv() noexcept {
		return (KcpUv&)uv;
	}
	KcpUv& KcpDialer::GetKcpUv() noexcept {
		return (KcpUv&)uv;
	}
	inline KcpUvUdpPeer::~KcpUvUdpPeer() {
		GetKcpUv().udpPeers.Remove(port);
	}
	inline KcpPeer::~KcpPeer() {
		GetKcpUv().kcpPeers.Remove(guid);
	}
}

int main(int argc, char* argv[]) {
	return 0;
}









//#include "xx_uv.h"
//
//struct KcpPeer {
//	ikcpcb *kcp;
//	std::chrono::time_point<std::chrono::steady_clock> lastTime;
//	std::function<int(const char *buf, int len)> OnOutput;
//	std::function<int(const char *buf, int len)> OnReceive;
//
//	KcpPeer(xx::Guid const& conv) {
//		kcp = ikcp_create(conv, this);
//		if (!kcp) throw - 1;
//		ikcp_nodelay(kcp, 1, 10, 2, 1);
//		ikcp_wndsize(kcp, 128, 128);
//		kcp->rx_minrto = 10;
//		ikcp_setoutput(kcp, [](const char *buf, int len, ikcpcb *kcp, void *user)->int {
//			return ((KcpPeer*)user)->OnOutput(buf, len);
//		});
//		lastTime = std::chrono::steady_clock::now();
//	}
//	~KcpPeer() {
//		ikcp_release(kcp);
//	}
//
//	int Input(const char *buf, int len) {
//		if (int r = ikcp_input(kcp, buf, len)) return r;
//		return 0;
//	}
//
//	int Send(const char *buf, int len) {
//		return ikcp_send(kcp, buf, len);
//	}
//
//	void Update(int ms) {
//		//auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - lastTime).count();
//		ikcp_update(kcp, ms);// (int)ms);
//		do {
//			char buf[512];
//			int recvLen = ikcp_recv(kcp, buf, 512);
//			if (recvLen <= 0) break;
//			OnReceive(buf, recvLen);
//		} while (true);
//	}
//};
//
//int main(int argc, char* argv[]) {
//	//xx::UvLoop loop;
//
//	xx::Guid conv;
//	auto server = xx::TryMake<KcpPeer>(conv);
//	auto client = xx::TryMake<KcpPeer>(conv);
//
//	server->OnOutput = [&](const char *buf, int len)->int {
//		return client->Input(buf, len);
//	};
//	client->OnOutput = [&](const char *buf, int len)->int {
//		return server->Input(buf, len);
//	};
//	server->OnReceive = [&](const char *buf, int len)->int {
//		return server->Send(buf, len);	// echo
//	};
//	client->OnReceive = [&](const char *buf, int len)->int {
//		std::cout << len;
//		return 0;
//	};
//
//	for (int i = 0; i < 1000; i++)
//	{
//		client->Send("a", 100);
//		server->Update(i * 10);
//		client->Update(i * 10);
//	}
//
//	//auto timer = xx::TryMake<xx::UvTimer>(loop, 0, 10);
//	//timer->OnFire = [&server, timer] {	// hold memory
//	//	server->Update();
//	//};
//	//loop.Run();
//	return 0;
//}
