#include "xx_uv.h"
#include <iostream>
#include <unordered_set>

struct Listener;
using Listener_s = std::shared_ptr<Listener>;
struct Peer;
using Peer_s = std::shared_ptr<Peer>;
struct Dialer;
using Dialer_s = std::shared_ptr<Dialer>;

struct Loop : xx::Uv {
	int addr = 0;			// for gen client peer addr
	std::unordered_map<int, Peer_s> clientPeers;
	std::unordered_map<int, Peer_s> serverPeers;
	std::shared_ptr<Listener> listener;
	xx::UvTimer_s timer;	// for auto dial
	std::unordered_map<int, Dialer_s> dialers;
	int RegisterDialer(int const& addr, std::string&& ip, int const& port) noexcept;

	Loop();
};

// 包结构: addr(4), data( serial + pkg )
struct Peer : xx::UvTcpBasePeer {
	Loop* loop = nullptr;
	std::unordered_map<int, Peer_s>* peers = nullptr;	// for find target peer
	int addr = 0;

	using xx::UvTcpBasePeer::UvTcpBasePeer;
	Peer(Peer const&) = delete;
	Peer& operator=(Peer const&) = delete;

	virtual int HandlePack(uint8_t* const& recvBuf, uint32_t const& recvLen) noexcept {
		loop->recvBB.Reset(recvBuf, recvLen, 0);

		decltype(addr) tar = 0;
		if (int r = loop->recvBB.ReadFixed(tar)) return r;

		auto iter = peers->find(tar);
		if (iter != peers->end()) {
			if (loop->recvBB.offset == recvLen) {		// 如果是握手包( 4字节长度 ), 获取 ip 发给目标服务器
				loop->sendBB.Clear();
				loop->sendBB.WriteFixed(addr);
				loop->sendBB.Write((uint8_t)0, (uint8_t)0);	// serial, typeId
				// todo: Write( GetIP() );						// \0 结尾字串
				return iter->second->Send(loop->sendBB.buf, loop->sendBB.len);
			}
			else {
				memcpy(recvBuf, &addr, sizeof(addr));	// replace
				return iter->second->Send(recvBuf, recvLen);
			}
		}
		// todo: else 地址不存在的回执通知?
		return 0;
	}
};

struct Dialer : xx::UvTcpDialer<Peer> {
	using BaseType = xx::UvTcpDialer<Peer>;
	using BaseType::BaseType;
	Loop* loop = nullptr;
	int addr = 0;
	std::string ip;
	int port = 0;
	inline int Dial() {
		return this->UvTcpDialer::Dial(ip, port, 2000);
	}

	inline virtual void Connect() noexcept {
		peer->addr = addr;
		peer->peers = &loop->serverPeers;
		peer->loop = loop;
		loop->serverPeers[peer->addr] = std::move(peer);
	}
};

struct Listener : xx::UvTcpListener<Peer> {
	using BaseType = xx::UvTcpListener<Peer>;
	Loop* loop = nullptr;

	using BaseType::BaseType;
	Listener(Listener const&) = delete;
	Listener& operator=(Listener const&) = delete;

	inline virtual void Accept(std::shared_ptr<Peer>& peer) noexcept override {
		peer->addr = ++loop->addr;
		peer->peers = &loop->clientPeers;
		peer->loop = loop;
		loop->clientPeers[peer->addr] = std::move(peer);
	};
};

inline Loop::Loop()
	: Uv() {
	listener = xx::TryMake<Listener>(*this, "0.0.0.0", 10000);
	if (listener) {
		listener->loop = this;
		std::cout << "router started...";
	}

	timer = xx::Make<xx::UvTimer>(*this);
	if (int r = timer->Start(100, 500, [this] {
		//if (dialers.empty()) {
		//	listener.reset();
		//	timer.reset();
		//	return;
		//}
		for (decltype(auto) kv : dialers) {
			if (!serverPeers[kv.first] && !kv.second->State()) {
				kv.second->Dial();
			}
		}
	})) throw r;
}

inline int Loop::RegisterDialer(int const& addr, std::string&& ip, int const& port) noexcept {
	if (dialers.find(addr) == dialers.end()) return -1;
	auto dialer = xx::TryMake<Dialer>(*this);
	if (!dialer) return -2;
	dialer->addr = addr;
	dialer->ip = std::move(ip);
	dialer->port = port;
	dialers[addr] = dialer;
	return 0;
}

int main() {
	Loop loop;
	// todo: RegisterDialer
	loop.Run();
	std::cout << "router stopd.";
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
