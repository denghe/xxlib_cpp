#include "xx_uv.h"
#include <unordered_set>
#include <chrono>
#include <iostream>
#include <thread>

struct EchoServerPeer : xx::UvTcpPeer {
	using BaseType = xx::UvTcpPeer;
	using BaseType::BaseType;

	std::shared_ptr<EchoServerPeer> holder;
	inline virtual void Disconnect() noexcept override {
		holder.reset();
	}
	inline virtual int ReceivePush(xx::Object_s&& msg) noexcept override {
		return SendPush(msg);
	}
	inline virtual int ReceiveRequest(int const& serial, xx::Object_s&& msg) noexcept override {
		return SendResponse(serial, msg);
	}
};
struct EchoPeerListener : xx::UvTcpListener<EchoServerPeer> {
	using BaseType = xx::UvTcpListener<EchoServerPeer>;
	using BaseType::BaseType;

	inline virtual void Accept(std::shared_ptr<EchoServerPeer>& peer) noexcept override {
		peer->holder = peer;
	}
};


struct EchoDialerPeer : xx::UvTcpPeer {
	using BaseType = xx::UvTcpPeer;
	using BaseType::BaseType;

	int counter = 0;
	std::chrono::time_point<std::chrono::system_clock> t = std::chrono::system_clock::now();
	int HandleMsg(xx::Object_s&& msg) {
		if (!msg) return -1;
		//xx::Cout(msg);
		if (++counter > 100000) {
			std::cout << double(std::chrono::nanoseconds(std::chrono::system_clock::now() - t).count()) / 1000000000 << std::endl;
			return -1;
		}
		return SendRequest(msg, [this](xx::Object_s&&msg) {
			return HandleMsg(std::move(msg));
		}, 2000);
	}
};
struct EchoDialer : xx::UvTcpDialer<EchoDialerPeer> {
	using BaseType = xx::UvTcpDialer<EchoDialerPeer>;
	using BaseType::BaseType;

	inline virtual void Connect() noexcept override {
		if (!peer) {
			std::cout << "dial timeout.\n";
			return;
		}
		auto msg = xx::Make<xx::BBuffer>();
		msg->Write(1u, 2u, 3u, 4u, 5u);
		peer->HandleMsg(std::move(msg));
	}
};


int main() {
	std::thread t1([] {
		xx::Uv uv;
		auto listener = xx::Make<EchoPeerListener>(uv, "0.0.0.0", 12345);
		uv.Run();
		std::cout << "server end.\n";
	});
	std::thread t2([] {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		xx::Uv uv;
		auto client = xx::Make<EchoDialer>(uv);
		client->Dial("127.0.0.1", 12345, 5000);
		uv.Run();
		std::cout << "client end.\n";
	});
	t1.join();
	t2.join();
	return 0;
}









































//#include "xx_uv_cpp.h"
//#include "xx_uv_cpp_echo.h"
//#include "xx_uv_cpp_package.h"
//
//int main() {
//	TestEcho();
//	return 0;
//}
//
////struct Pos {
////	double x = 0, y = 0;
////};
////template<>
////struct BFuncs<Pos, void> {
////	static inline void WriteTo(BBuffer& bb, Pos const& in) noexcept {
////		bb.Write(in.x, in.y);
////	}
////	static inline int ReadFrom(BBuffer& bb, Pos& out) noexcept {
////		return bb.Read(out.x, out.y);
////	}
////};
//
//#include "xx_bbuffer.h"
//#include <iostream>
//
//struct Node : Object {
//	int indexAtContainer;
//	std::weak_ptr<Node> parent;
//	std::vector<std::weak_ptr<Node>> childs;
//#pragma region
//	inline virtual uint16_t GetTypeId() const noexcept override {
//		return 3;
//	}
//	inline virtual void ToBBuffer(BBuffer& bb) const noexcept override {
//		bb.Write(this->indexAtContainer, this->parent, this->childs);
//	}
//	inline virtual int FromBBuffer(BBuffer& bb) noexcept override {
//		return bb.Read(this->indexAtContainer, this->parent, this->childs);
//	}
//#pragma endregion
//};
//
//struct Container : Object {
//	std::vector<std::shared_ptr<Node>> nodes;
//	std::weak_ptr<Node> node;
//#pragma region
//	inline virtual uint16_t GetTypeId() const noexcept override {
//		return 4;
//	}
//	inline virtual void ToBBuffer(BBuffer& bb) const noexcept override {
//		bb.Write(this->nodes, this->node);
//	}
//	inline virtual int FromBBuffer(BBuffer& bb) noexcept override {
//		return bb.Read(this->nodes, this->node);
//	}
//#pragma endregion
//};
//
//int main() {
//	BBuffer::Register<Node>(3);
//	BBuffer::Register<Container>(4);
//
//	auto c = std::make_shared<Container>();
//	auto n = std::make_shared<Node>();
//	n->indexAtContainer = c->nodes.size();
//	c->nodes.push_back(std::move(n));
//	n = std::make_shared<Node>();
//	n->indexAtContainer = c->nodes.size();
//	c->nodes.push_back(std::move(n));
//	c->node = c->nodes[0];
//	c->node.lock()->parent = c->node;
//	c->node.lock()->childs.push_back(c->node);
//	c->node.lock()->childs.push_back(c->nodes[1]);
//
//	BBuffer bb;
//	bb.WriteRoot(c);
//	std::cout << bb.ToString() << std::endl;
//
//	std::shared_ptr<Container> tmp;
//	int r = bb.ReadRoot(tmp);
//	assert(!r);
//	auto c2 = std::dynamic_pointer_cast<Container>(tmp);
//	assert(c2);
//	std::cout << c2->nodes.size()
//		<< " " << (c2->node.lock() == c2->nodes[0])
//		<< " " << (c2->node.lock() == c2->node.lock()->parent.lock())
//		<< " " << c2->node.lock()->childs.size()
//		<< " " << (c2->node.lock()->childs[0].lock() == c2->node.lock())
//		<< " " << (c2->nodes[1] == c2->node.lock()->childs[1].lock())
//		<< std::endl;
//
//	return 0;
//}
