#include "xx_uv.h"

struct EchoPeer : xx::UvKcpPeer {
	using BaseType = xx::UvKcpPeer;
	using BaseType::BaseType;

	std::shared_ptr<EchoPeer> holder;
	virtual void Disconnect() noexcept override {
		xx::Cout(guid, " disconnected.\n");
		holder = nullptr;					// release peer
	}
	virtual int ReceivePush(xx::Object_s&& msg) noexcept override {
		//xx::CoutN("recv push ", msg);
		if (int r = SendPush(msg)) return r;
		Flush();
		ResetTimeoutMS(3000);
		return 0;
	}
	virtual int ReceiveRequest(int const& serial, xx::Object_s&& msg) noexcept override {
		//xx::CoutN("recv request serial = ", serial, ", msg = ", msg);
		if (int r = SendResponse(serial, msg)) return r;
		Flush();
		ResetTimeoutMS(3000);
		return 0;
	}
};

struct EchoPeerListener : xx::UvKcpListener<EchoPeer> {
	using BaseType = xx::UvKcpListener<EchoPeer>;
	using BaseType::BaseType;

	inline virtual void Accept(std::shared_ptr<xx::UvKcpBasePeer>& peer_) noexcept override {
		auto&& peer = xx::As<EchoPeer>(peer_);
		peer->holder = peer;				// hold memory
		peer->ResetTimeoutMS(3000);			// 设置 3 秒内没收到能解析出来的合法包就 Dispose
		xx::Cout(peer->guid, " accepted.\n");
	}
};

int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cout << "need args: port\n";
		return -1;
	}
	xx::Uv uv;
	auto listener = xx::Make<EchoPeerListener>(uv, "0.0.0.0", std::atoi(argv[1]));
	uv.Run();
	return 0;
}
