#include "xx_uv.h"
#include "../gens/output/PKG_class.h"
#include <iostream>
#include <unordered_set>

namespace xx {

	struct RouterPeer;
	struct VPeer {
		int addr = -1;
		RouterPeer* peer = nullptr;
		std::string ip;
		std::function<int(Object_s&& msg)> OnReceivePush;
		std::function<int(int const& serial, Object_s&& msg)> OnReceiveRequest;
		std::function<void()> OnDisconnect;

		// todo: SendXxxxx though owner
		// todo: Dispose 时发起断线通知? 析构不发起?
	};
	using VPeer_s = std::shared_ptr<VPeer>;
	using VPeer_w = std::weak_ptr<VPeer>;


	struct RouterPeer : UvTcpPeer {
		std::unordered_map<int, VPeer_s> peers;
		std::function<VPeer_s()> OnCreateVPeer;
		std::function<int(VPeer_s&)> OnAcceptVPeer;

		RouterPeer() = default;
		RouterPeer(RouterPeer const&) = delete;
		RouterPeer& operator=(RouterPeer const&) = delete;

		virtual int HandlePack(uint8_t* const& recvBuf, uint32_t const& recvLen) noexcept override {
			if (recvLen < 6) return 0;
			auto& recvBB = uv.recvBB;
			recvBB.Reset((uint8_t*)recvBuf, recvLen);

			int addr = 0;
			if (int r = recvBB.ReadFixed(addr)) return r;

			VPeer* peer = nullptr;
			auto iter = peers.find(addr);
			if (iter != peers.end()) {
				if (recvLen == 6 && !recvBuf[4] && !recvBuf[5]) {
					//iter->second->Dispose(true);
					peers.erase(iter);
					return 0;
				}
				else {
					peer = &*iter->second; 
				}
			}
			else {
				if (recvLen <= 6 || recvBuf[4] || recvBuf[5]) return 0;		// addr(4 bytes), 0, 0, ip string\0
				auto p = OnCreateVPeer();
				if (!p) return 0;
				p->addr = addr;
				p->ip = (char*)recvBuf[6];
				if (OnAcceptVPeer(p)) {
					Send(recvBuf, 6);							// todo: 回复断线给对方?
					return 0;
				}
				p->peer = this;
				peers[addr] = p;
				peer = &*p;
			}

			int serial = 0;
			if (int r = recvBB.Read(serial)) return r;
			Object_s msg;
			if (int r = recvBB.ReadRoot(msg)) return r;

			if (serial == 0) {
				return peer->OnReceivePush ? peer->OnReceivePush(std::move(msg)) : 0;
			}
			else if (serial < 0) {
				return peer->OnReceiveRequest ? peer->OnReceiveRequest(-serial, std::move(msg)) : 0;
			}
			else {
				auto iter = callbacks.find(serial);
				if (iter == callbacks.end()) return 0;
				int r = iter->second.first(std::move(msg));
				callbacks.erase(iter);
				return r;
			}
		}
		virtual void Dispose(int const& flag = 0) noexcept override;
	};
	using RouterPeer_s = std::shared_ptr<RouterPeer>;
	using RouterPeer_w = std::weak_ptr<RouterPeer>;

	//struct RouterListener : UvTcpBaseListener {
	//	int idx = 0;
	//	std::unordered_map<int, RouterPeer_s> routerPeers;
	//	std::unordered_map<int, VPeer_s> peers;

	//	// todo: OnAccept( VPeer )

	//	RouterListener() = default;
	//	RouterListener(RouterListener const&) = delete;
	//	RouterListener& operator=(RouterListener const&) = delete;

	//	inline virtual std::shared_ptr<UvTcpBasePeer> CreatePeer() noexcept override {
	//		return xx::TryMake<RouterPeer>();
	//	}
	//	inline virtual void Accept(std::shared_ptr<UvTcpBasePeer>&& peer_) noexcept override {
	//		routerPeer = std::move(xx::As<RouterPeer>(peer_));
	//		peer->peers = &peers;
	//		peer->indexAtContainer = peers.len;
	//		peers.Add(std::move(peer));
	//	}
	//};

	//inline void RouterPeer::Dispose(int const& flag = 0) noexcept {
	//	if (!uvTcp) return;
	//	this->UvTcpBasePeer::Dispose(flag);
	//	peers.clear();
	//}

	//struct VDialer;
	//using VDialer_s = std::shared_ptr<VDialer>;
	//using VDialer_w = std::weak_ptr<VDialer>;

	//struct RouterDialer : UvTcpDialer {
	//	std::unordered_map<int, VDialer_s> dialers;
	//	std::unordered_map<int, VPeer_s> peers;
	//	inline virtual std::shared_ptr<UvTcpBasePeer> CreatePeer() noexcept override {
	//		auto peer = xx::TryMake<RouterPeer>();
	//		peer->peers = &peers;
	//		return peer;
	//	}
	//	inline virtual void Connect() noexcept override {
	//	}
	//};

	//struct VDialer {
	//	VPeer_s peer;
	//};
}

//void RunServer1() {
//	xx::UvLoop loop;
//	std::unordered_set<xx::UvTcpPeer_s> peers;
//	auto listener = loop.CreateTcpListener<xx::RouterListener>("0.0.0.0", 10001);
//	assert(listener);
//	listener->OnAccept = [&peers](xx::UvTcpPeer_s&& peer) {
//		peer->OnReceiveRouteRequest = [peer_w = xx::UvTcpPeer_w(peer)](int const& addr, int const& serial, xx::Object_s&& msg)->int {
//			return peer_w.lock()->SendResponse(serial, msg, addr);
//		};
//		peer->OnDisconnect = [&peers, peer_w = xx::UvTcpPeer_w(peer)]{
//			peers.erase(peer_w.lock());
//		};
//		peers.insert(std::move(peer));
//	};
//	loop.Run();
//	std::cout << "server end.\n";
//}

void RunRouter() {

}

int main() {
	return 0;
}































//#define var decltype(auto)

//struct GameEnv {
//	std::unordered_map<int, PKG::Player_s> players;
//	PKG::Scene_s scene;
//};
//
//int main() {
//	PKG::AllTypesRegister();
//
//	GameEnv env;
//	xx::MakeShared(env.scene);
//	xx::MakeShared(env.scene->monsters);
//	xx::MakeShared(env.scene->players);
//
//	var p1 = PKG::Player::MakeShared();
//	var p2 = PKG::Player::MakeShared();
//	var m1 = PKG::Monster::MakeShared();
//	var m2 = PKG::Monster::MakeShared();
//
//	p1->id = 123;
//	xx::MakeShared(p1->token, "asdf");
//	p1->target = m1;
//
//	p2->id = 234;
//	xx::MakeShared(p1->token, "qwer");
//	p2->target = m2;
//
//	env.players[p1->id] = p1;
//	env.players[p2->id] = p2;
//
//	env.scene->monsters->Add(m1, m2);
//	env.scene->players->Add(p1, p2);
//
//	std::cout << env.scene << std::endl;
//
//	xx::BBuffer bb;
//	{
//		var sync = PKG::Sync::MakeShared();
//		xx::MakeShared(sync->players);
//		for (var player_w : *env.scene->players) {
//			if (var player = player_w.lock()) {
//				sync->players->Add(player);
//			}
//		}
//		sync->scene = env.scene;
//		bb.WriteRoot(sync);
//	}
//	std::cout << bb << std::endl;
//
//	//env.players
//	return 0;
//}
//

























//int main() {
//	PKG::AllTypesRegister();
//	std::cout << PKG::PkgGenMd5::value << std::endl;
//
//	auto o = xx::TryMake<PKG::Foo>();
//	o->parent = o;
//	o->childs = o->childs->Create();
//	o->childs->Add(o);
//	std::cout << o << std::endl;
//
//	xx::BBuffer bb;
//	bb.WriteRoot(o);
//	std::cout << bb << std::endl;
//
//	auto o2 = PKG::Foo::Create();
//	int r = bb.ReadRoot(o2);
//	std::cout << r << std::endl;
//	std::cout << o2 << std::endl;
//	
//	return 0;
//}
