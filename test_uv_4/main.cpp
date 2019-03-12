#include "xx_uv.h"
struct MyClient : xx::UvTcpClient {
	using xx::UvTcpClient::UvTcpClient;
	int counter = 0;
	std::chrono::time_point<std::chrono::system_clock> t = std::chrono::system_clock::now();
	void HandleMsg(xx::Object_p& msg) {
		if (!msg) return;
		if (++counter > 100000) {
			std::cout << double(std::chrono::nanoseconds(std::chrono::system_clock::now() - t).count()) / 1000000000 << std::endl;
			return;
		}
		SendRequestEx(msg, [this](xx::Object_p& msg) {
			return HandleMsg(std::move(msg));
		}, 1000);
	}
};

int main() {
	xx::MemPool::RegisterInternals();
	std::thread t1([] {
		xx::MemPool mp;
		xx::UvLoop uvloop(&mp);
		uvloop.InitRpcTimeoutManager();
		auto listener = uvloop.CreateTcpListener();
		listener->Bind("0.0.0.0", 12345);
		listener->Listen();
		listener->OnAccept = [](xx::UvTcpPeer_w peer) {
			peer->OnReceiveRequest = [peer](uint32_t serial, xx::BBuffer& bb) {
				xx::Object_p o;
				if (bb.ReadRoot(o)) return;
				peer->SendResponse(serial, o);
			};
		};
		uvloop.Run();
		std::cout << "server end.";
	});
	std::thread t2([] {
		xx::MemPool mp;
		xx::UvLoop uvloop(&mp);
		uvloop.InitRpcTimeoutManager();
		int counter = 0;
		std::chrono::time_point<std::chrono::system_clock> t = std::chrono::system_clock::now();
		auto client = mp.CreatePtr<MyClient>(uvloop);
		client->ConnectEx("127.0.0.1", 12345);
		client->OnConnect = [&](int status) {
			if (status) return;
			auto msg = mp.MPCreatePtr<xx::BBuffer>();
			msg->Write(1u, 2u, 3u, 4u, 5u);
			client->SendRequestEx(msg, [&](xx::Object_p& msg) {
				client->HandleMsg(msg);
			}, 1000);
		};
		
		uvloop.Run();
		std::cout << "client end.";
	});
	t1.join();
	t2.join();
	return 0;
}
