#pragma once
#include <functional>
#include <vector>

namespace xx {

	struct Stackless {
		using FuncType = std::function<int(int const& lineNumber)>;
		std::vector<std::pair<FuncType, int>> funcs;
		inline void Add(FuncType&& func) {
			if (!func) return;
			funcs.emplace_back(std::move(func), 0);
		}
		inline void RunAdd(FuncType&& func) {
			if (!func) return;
			int n = func(0);
			if (n == (int)0xFFFFFFFF) return;
			funcs.emplace_back(std::move(func), n);
		}
		size_t RunOnce() {
			if (funcs.size()) {
				for (decltype(auto) i = funcs.size() - 1; i != (size_t)-1; --i) {
					decltype(auto) func = funcs[i];
					if (func.second < 0) {
						++func.second;
					}
					else {
						func.second = func.first(func.second);
					}
					if (func.second == (int)0xFFFFFFFF) {
						if (i + 1 < funcs.size()) {
							funcs[i] = std::move(funcs[funcs.size() - 1]);
						}
						funcs.pop_back();
					}
				}
			}
			return funcs.size();
		}
	};

#define COR_BEGIN	switch (lineNumber) { case 0:
#define COR_YIELD	return __LINE__; case __LINE__:;
#define COR_END		} return (int)0xFFFFFFFF;

}

/*
UvLoop loop;
Stackless cors;
struct Ctx1 {
	std::shared_ptr<TcpClient> client;
	std::chrono::system_clock::time_point t;
};
cors.Add([&, zs = std::make_shared<Ctx1>()](int const& lineNumber) {
	switch (lineNumber) {
	case 0:
		zs->client = loop.CreateClient<TcpClient>();
	case 1:
	LabConnect:
		xx::Cout("connecting...\n");
		zs->client->Cleanup();
		zs->client->Dial("127.0.0.1", 12345);
		zs->t = std::chrono::system_clock::now() + std::chrono::seconds(5);
		while (!zs->client->peer) {
			return 2; case 2:;
			if (std::chrono::system_clock::now() > zs->t) {		// timeout check
				xx::Cout("timeout. retry\n");
				goto LabConnect;
			}
		}
		xx::Cout("connected. send 'a'\n");
		zs->client->peer->Send("a", 1);

		while (!zs->client->peer->Disposed()) {
			return 3; case 3:;
		}
		goto LabConnect;
	}
	return (int)0xFFFFFFFF;
});

auto timer = loop.CreateTimer(0, 1000 / 61);
timer->OnFire = [&] {
	if (!cors.RunOnce()) {
		timer->Dispose();
	};
};
loop.Run();
*/