#pragma once
#include "xx_uv.h"
#include "xx_uv_coroutine.h"
#include <unordered_set>
#include <chrono>
#include <iostream>
#include <thread>

UvLoopCoroutine loop(60);

bool GetIPList(Yield& yield, std::vector<std::string>& rtv, std::string const& domainName, double const& timeoutSec) {
	auto resolver = loop.CreateResolver();
	if (!resolver) return false;
	int state = 0;
	resolver->OnTimeout = [&] {
		state = 1;
	};
	resolver->OnFinish = [&] {
		rtv = std::move(resolver->ips);
		state = 2;
	};
	resolver->Resolve(domainName, (int64_t)(timeoutSec * 1000));
	while (!state) yield();
	return state == 2;
}

std::shared_ptr<UvTcpBasePeer> Dial(Yield& yield, std::vector<std::string> const& ipList, double const& timeoutSec) {
	std::shared_ptr<UvTcpBasePeer> rtv;
	auto client = loop.CreateClient<UvTcpBaseClient>();
	if (!client) return nullptr;
	int state = 0;
	client->OnTimeout = [&] {
		state = 1;
	};
	client->OnConnect = [&] {
		rtv = std::move(client->peer);
		state = 2;
	};
	client->Dial(ipList, 80, (int64_t)(timeoutSec * 1000));
	while (!state) yield();
	return rtv;
}

void TestUvCoroutine() {
	loop.Add(Coroutine([&](Yield& yield) {
	LabResolve:
		std::vector<std::string> ipList;
		if (GetIPList(yield, ipList, "www.baidu.com", 1000)) {
			std::cout << ipList.size() << std::endl;
			for (decltype(auto) ip : ipList) {
				std::cout << ip << std::endl;
			}
		}
		else {
			std::cout << "resolve domain timeout.";
			goto LabResolve;
		}
	LabDial:
		auto peer = Dial(yield, ipList, 1000);
		if (!peer) {
			Sleep(yield, std::chrono::seconds(1));
			goto LabDial;
		}
		std::cout << "connected.";

		peer->OnReceive = [](uint8_t const* const& buf, uint32_t const& len)->int {
			for (size_t i = 0; i < len; i++) {
				std::cout << (char)buf[i];
			}
			return 0;
		};
		std::string httpGet = "GET / HTTP/1.1\r\n\r\n";
		peer->Send((uint8_t*)httpGet.data(), httpGet.size());

		while (!peer->Disposed()) yield();
		Sleep(yield, std::chrono::seconds(1));
		goto LabDial;
	}));
	loop.Run();
}
