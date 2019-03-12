#pragma once
#include "xx_uv.h"
#include "xx_uv_stackless.h"
#include <iostream>

void TestUvStackless() {
	UvLoopStackless loop(61);
	struct Ctx1 {
		std::shared_ptr<UvTcpPackClient> client;
		std::shared_ptr<UvTcpPackPeer> peer;
		std::chrono::system_clock::time_point t;
		std::vector<Buffer> recvs;
		int count = 0;
	};
	loop.Add([&, zs = std::make_shared<Ctx1>()](int const& lineNumber) {
		COR_BEGIN
			zs->client = loop.CreateClient<UvTcpPackClient>();
	LabConnect:
		if (++zs->count > 3) goto LabEnd;
		std::cout << "connecting...\n";
		zs->client->Dial("127.0.0.1", 12345);
		zs->t = std::chrono::system_clock::now() + std::chrono::seconds(2);
		while (!zs->client->peer) {
			COR_YIELD
				if (std::chrono::system_clock::now() > zs->t) {		// timeout check
					std::cout << "timeout. retry\n";
					goto LabConnect;
				}
		}
		std::cout << "connected.\n";
		zs->peer = std::move(zs->client->Peer<UvTcpPackPeer>());
		zs->peer->OnReceivePack = [wzs = std::weak_ptr<Ctx1>(zs)](uint8_t const* const& buf, uint32_t const& len) {
			if (auto zs = wzs.lock()) {
				Buffer b(len);
				b.Append(buf, len);
				zs->recvs.push_back(std::move(b));
			}
			return 0;
		};
		zs->recvs.clear();

	LabSend:
		std::cout << "send 'asdf'\n";
		zs->peer->SendPack((uint8_t*)"asdf", 4);

		std::cout << "wait echo & check state\n";
		while (!zs->peer->Disposed()) {
			COR_YIELD
				if (zs->recvs.size()) {
					for (decltype(auto) b : zs->recvs) {
						std::cout << std::string((char*)b.buf, b.len) << std::endl;
					}
					zs->recvs.clear();
					goto LabSend;
				}
		}
		std::cout << "disconnected. retry\n";
		goto LabConnect;
	LabEnd:;
		COR_END
	});
	loop.Run();
	std::cout << "end. \npress any key to continue...\n";
	std::cin.get();
}
