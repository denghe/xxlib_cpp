#pragma execution_character_set("utf-8")
#define _CRT_SECURE_NO_WARNINGS
#include "xx_uv.h"
#include "xx_pos.h"
#include "xx_random.h"
#include "PKG_class.h"
#include "PKG_class.hpp"
#include "xx_random.hpp"
#include <random>
#include <tuple>

namespace std {
	template<>
	struct hash<std::tuple<int, int, int>> {
		std::size_t operator()(std::tuple<int, int, int> const& in) const noexcept {
			static_assert(sizeof(std::size_t) >= 8);
			return (std::size_t)std::get<0>(in) | ((std::size_t)std::get<2>(in) << 32);	// 1 是 cannonId 极少冲突
		}
	};
}

// todo: 增强断线重连或消息重发的兼容性。
// game_server 在连接上来之后，需要先汇报自己的 id，无脑信任。 id 与 peer 对应起来，如果 peer 对应已存在，则踢掉旧的, 换新的 peer
// 每个计算请求附带一个编号，该编号与 id peer 对应起来，每次收到请求后更新，如果收到重复的或更小的编号，则补发 cache 数据, 不再重复计算

struct Service {
	xx::Uv uv;
	xx::UvListener_s listener;

	// 计算服务至少具备 总输入 & 输出 两项持久化累加值, 以便根据胜率曲线配型, 决策输赢. 
	// 控制的目的是令实际输赢尽量贴近设计的曲线, 达到可控范围内的大起大落效果.
	// 就曲线图来讲, x 值为总输入, y 值为胜率. 需要配置这样一张配置表( 机器人有另外一套胜率表 )
	// 当总输入 > 总输出 / 胜率 时, 差值将对胜率起到放大作用. 反之则缩小.
	// 长时间后最终盈利 = (输入 - 输出) + (机器人总得 - 机器人总押)
	int64_t totalInput = 0;
	int64_t totalOutput = 0;

	// 随机数发生器
	std::mt19937_64 rnd;

	// 先写死. 模拟读取到一行胜率配置. min max 控制波幅
	double ratio = 0.9;
	double maxRatio = 1.1;
	double minRatio = 0.95;

	// 这里采用流式计算法, 省去两端组织 & 还原收发的数据. 每发生一次碰撞, 就产生一条 Hit 请求. 
	// 当一帧的收包 & 处理阶段结束后, 将产生的 Hit 队列打包发送给 Calc 服务计算.
	// Calc 依次处理, 以 fishId & playerId + bulletId 做 key, 逐个判断碰撞击打的成败. 
	// key: fishId, value: Hit* 如果鱼已死, 则放入该字典. 存当前 Hit 数据以标识打死该鱼的玩家 & 子弹
	// key: pair<playerId, bulletId>, value: bulletCount, bulletCoin 如果子弹数量有富余( 鱼死了, 没用完 ), 就会创建一行记录来累加
	// 最后将这两个字典的结果序列化返回
	xx::Dict<int, PKG::Calc::CatchFish::Hit const*> fishs;
	xx::Dict<std::tuple<int, int, int>, std::pair<int, int64_t>> bullets;
	PKG::Calc_CatchFish::HitCheckResult_s hitCheckResult;

	// ...
	// ...

	inline int OnReceiveRequest(xx::UvPeer_s const& peer, int const& serial, xx::Object_s&& msg) {
		switch (msg->GetTypeId()) {
		case xx::TypeId_v<PKG::CatchFish_Calc::Push>:
			return HandleRequest(peer, serial, xx::As<PKG::CatchFish_Calc::Push>(msg));
		case xx::TypeId_v<PKG::CatchFish_Calc::Pop>:
			return HandleRequest(peer, serial, xx::As<PKG::CatchFish_Calc::Pop>(msg));
		case xx::TypeId_v<PKG::CatchFish_Calc::HitCheck>:
			return HandleRequest(peer, serial, xx::As<PKG::CatchFish_Calc::HitCheck>(msg));
			// ...
			// ...
		default:
			return -1;	// unknown package type received.
		}
		return 0;
	}

	inline int HandleRequest(xx::UvPeer_s const& peer, int const& serial, PKG::CatchFish_Calc::Push_s&& msg) {
		totalInput += msg->value;
		// todo: log
		return 0;
	}

	inline int HandleRequest(xx::UvPeer_s const& peer, int const& serial, PKG::CatchFish_Calc::Pop_s&& msg) {
		totalOutput += msg->value;
		// todo: log
		return 0;
	}

	inline int HandleRequest(xx::UvPeer_s const& peer, int const& serial, PKG::CatchFish_Calc::HitCheck_s&& msg) {
		// 依次处理所有 hit
		for (auto&& hit : *msg->hits) {
			Handle_Hit(hit);
		}

		// 扫描 & 填充输出结果
		for (auto&& o : fishs) {
			auto&& t = hitCheckResult->fishs->Emplace();
			auto&& f = *o.value;
			t.fishId = f.fishId;
			t.playerId = f.playerId;
			t.cannonId = f.cannonId;
			t.bulletId = f.bulletId;
			t.fishCoin = f.fishCoin;
			t.bulletCoin = f.bulletCoin;
		}
		for (auto&& o : bullets) {
			auto&& t = hitCheckResult->bullets->Emplace();
			t.playerId = std::get<0>(o.key);
			t.cannonId = std::get<1>(o.key);
			t.bulletId = std::get<2>(o.key);
			t.bulletCount = o.value.first;
			t.bulletCoin = o.value.second;
		}

		// 发送
		peer->SendResponse(serial, hitCheckResult);

		// 各式 cleanup
		hitCheckResult->fishs->Clear();
		hitCheckResult->bullets->Clear();
		fishs.Clear();
		bullets.Clear();
		return 0;
	}

	inline void Handle_Hit(PKG::Calc::CatchFish::Hit const& hit) {
		// 如果 fishId 已存在: 该 fish 已死, 子弹退回剩余次数
		if (fishs.Find(hit.fishId) != -1) {
			auto&& v = bullets[std::make_tuple(hit.playerId, hit.cannonId, hit.bulletId)];
			v.first += hit.bulletCount;
			v.second = hit.bulletCoin;
			return;
		}
		// 根据子弹数量来多次判定, 直到耗光或鱼死中断
		for (int i = 0; i < hit.bulletCount; ++i) {
			// 进行鱼死判定. 如果死掉：记录下来, 剩余子弹退回
			if (FishDieCheck(hit)) {
				fishs[hit.fishId] = &hit;
				// 如果没有剩余数量就不记录了
				if (auto && left = hit.bulletCount - i - 1) {	// - 1: 本次的扣除
					auto&& v = bullets[std::make_tuple(hit.playerId, hit.cannonId, hit.bulletId)];
					v.first += left;
					v.second = hit.bulletCoin;
				}
				return;
			}
			// else 子弹消耗掉了，鱼没死，啥都不记录
		}
	}

	// 进行一次鱼死判定, 同时更新总输入输出
	inline bool FishDieCheck(PKG::Calc::CatchFish::Hit const& hit) {
		totalInput += hit.bulletCoin;
		auto r = (totalInput > totalOutput && double(totalOutput) / double(totalInput) < ratio) ? maxRatio : minRatio;
		auto b = std::uniform_real_distribution(0.0, double(hit.fishCoin))(rnd);

		// 临时打印一下看看
		xx::CoutTN("totalInput = ", totalInput, ", totalOutput = ", totalOutput, ", fix = ", r, /*", fish.coin = ", hit.fishCoin, ", rnd = ", b,*/ ", cfg ratio = ", ratio, ", curr ratio = ",(double(totalOutput) / double(totalInput)) );

		if (b <= r) {
			totalOutput += hit.fishCoin * hit.bulletCoin;
			return true;
		}
		return false;
	}

	Service() {
		// todo: fill totalInput & totalOutput
		// todo: win ratio config

		rnd.seed(std::random_device()());

		xx::MakeTo(hitCheckResult);
		xx::MakeTo(hitCheckResult->fishs);
		xx::MakeTo(hitCheckResult->bullets);

		xx::MakeTo(listener, uv, "0.0.0.0", 12333, 0);
		listener->onAccept = [this](xx::UvPeer_s p) {
			xx::CoutTN("accept: ", p->GetIP());
			p->onDisconnect = [p] {
				xx::CoutTN("disconnect: ", p->GetIP());
			};
			p->onReceiveRequest = [this, p](auto s, auto m) { return OnReceiveRequest(p, s, std::move(m)); };
		};
	}

	inline void Run() {
		xx::CoutTN("Calc service running...");
		uv.Run();
	}
};

int main() {

	//{
	//	int64_t totalInput = 0;
	//	int64_t totalOutput = 0;

	//	std::mt19937_64 rnd;
	//	std::mt19937_64 rnd2;

	//	double ratio = 0.9;
	//	double maxRatio = 1.10;
	//	double minRatio = 0.90;


	//	while (true) {
	//		auto bulletCoin = std::uniform_int_distribution(1, 1000)(rnd2);
	//		auto fishCoin = std::uniform_int_distribution(5, 500)(rnd2);

	//		totalInput += bulletCoin;
	//		auto r = (totalInput > totalOutput && double(totalOutput) / double(totalInput) < ratio) ? maxRatio : minRatio;
	//		auto b = std::uniform_real_distribution(0.0, (double)fishCoin)(rnd);

	//		// 临时打印一下看看
	//		xx::CoutTN("totalInput = ", totalInput, ", totalOutput = ", totalOutput, ", fix = ", r, /*", fish.coin = ", hit.fishCoin, ", rnd = ", b,*/ ", cfg ratio = ", ratio, ", curr ratio = ", (double(totalOutput) / double(totalInput)));

	//		if (b <= r) {
	//			totalOutput += fishCoin * bulletCoin;
	//		}
	//	}
	//}



	std::shared_ptr<Service> service;
	try {
		xx::MakeTo(service);
	}
	catch (int const& r) {
		xx::CoutTN("create Calc service throw exception: r = ", r);
	}
	service->Run();
	return 0;
}
