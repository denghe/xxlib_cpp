#include "PKG_CatchFish_Scene_MakeFishs.hpp"

#ifdef CC_TARGET_PLATFORM
inline int PKG::CatchFish::Scene::InitCascade(void* const& o) noexcept {
	// 将 scene 指针刷下去保存在类里面以便于使用
	// 备份，不做 InitCascade
	auto borns = std::move(this->borns);
	// 下刷
	int r = InitCascadeCore(this);
	// 还原
	this->borns = std::move(borns);
	return r;
}
#endif

inline int PKG::CatchFish::Scene::Update() noexcept {
#ifndef CC_TARGET_PLATFORM
	// 如果到计算服务的连接未就绪, 无限等待			// todo: 超时处理
	if (!service->IsAlive_CalcPeer()) {
		xx::CoutTN("waiting Calc service...");
		return 0;
	}

	// 如果正在等待计算服回调, 就直接返回. ( 正常情况下不该发生. 除非与 Calc 连接异常 )
	if (calcResult) return calcResult > 0 ? 0 : calcResult;
#endif

	// 一开始就累加帧数, 确保后续步骤( 含追帧递归 )生命周期正确
	++frameNumber;

	// 遍历更新. 倒序扫描, 交换删除. 如果存在内部乱序删除的情况, 则需要 名单机制 或 标记机制 在更新结束之后挨个删掉
	auto&& fs = *fishs;
	if (fs.len) {
		for (size_t i = fs.len - 1; i != -1; --i) {
			assert(fs[i]->indexAtContainer == (int)i);
			if (fs[i]->Update(frameNumber)) {
				fs[fs.len - 1]->indexAtContainer = (int)i;
				fs.SwapRemoveAt(i);
			}
		}
	}

	// 倒序遍历 items. Update 返回非 0 则杀掉
	auto&& is = *items;
	if (is.len) {
		for (size_t i = is.len - 1; i != -1; --i) {
			if (int r = is[i]->Update(frameNumber)) {
				is.SwapRemoveAt(i);
			}
		}
	}

	// 如果关卡持续时间到达就循环切换到下一关
	assert(stage);
	assert(stage->ticks <= stage->cfg_endTicks);
	if (stage->ticks == stage->cfg_endTicks) {
		int stageId = stage->cfg_id == cfg->stages->len - 1 ? 0 : stage->cfg_id + 1;
		cfg->stageBufs[stageId].offset = 0;
		int r = cfg->stageBufs[stageId].ReadRoot(stage);
		assert(!r);
		stage->InitCascade(this);
	}

	// 取出关卡 ticks 备用
	auto&& ticks = ++stage->ticks;

	// 倒序遍历发射器. 判断生效时间. Update 返回非 0 则杀掉
	auto&& es = *stage->elements;
	if (es.len) {
		for (size_t i = es.len - 1; i != -1; --i) {
			auto&& e = es[i];
			if (e->cfg_endTicks <= ticks) {
				es.SwapRemoveAt(i);
				continue;
			}
			if (e->cfg_beginTicks <= ticks) {
				if (int r = e->Update(ticks)) {
					es.SwapRemoveAt(i);
					continue;
				}
			}
		}
	}

#ifndef CC_TARGET_PLATFORM
	// 倒序遍历监视器. 判断生效时间. Update 返回非 0 则杀掉
	auto&& ms = *stage->monitors;
	if (ms.len) {
		for (size_t i = ms.len - 1; i != -1; --i) {
			auto&& m = ms[i];
			if (m->cfg_endTicks <= ticks) {
				ms.SwapRemoveAt(i);
				continue;
			}
			if (m->cfg_beginTicks <= ticks) {
				if (int r = m->Update(ticks)) {
					ms.SwapRemoveAt(i);
					continue;
				}
			}
		}
	}
#endif

	// 倒序遍历预约生成鱼. 时间到达就将鱼移到 fishs 并初始化相关数据
	auto&& bs = *borns;
	if (bs.len) {
		for (size_t i = bs.len - 1; i != -1; --i) {
			auto&& b = bs[i];
			assert(b->beginFrameNumber >= frameNumber);
			if (b->beginFrameNumber == frameNumber) {
				b->fish->indexAtContainer = (int)fs.len;
				fs.Add(b->fish);
#ifdef CC_TARGET_PLATFORM
				b->fish->InitCascade(this);
#endif
				bs.SwapRemoveAt(i);
			}
		}
	}

	// 倒序遍历玩家，Update 返回非 0 则杀掉( 玩家会进一步驱动 cannons, bullets )
	auto&& ps = *players;
	if (ps.len) {
		for (size_t i = ps.len - 1; i != -1; --i) {
			auto w = ps[i];								// 后面 assert 要用. 中途可能被删掉. 不存 && 引用
			int r = 0;
			{
				auto&& p = xx::As<Player>(w.lock());
				if ((r = p->Update(frameNumber))) {
					catchFish->Cleanup(p);
				}
			}
			assert(!r || r && !w.lock());
		}
	}

#ifndef CC_TARGET_PLATFORM
	if (hitChecks->hits->len) {
		// 设置标志位
		calcResult = 1;

		// 将 hitChecks 发给 Calc 计算
		service->calcPeer->SendRequest(hitChecks, [this](xx::Object_s && msg)->int { UpdateCalc(std::move(msg)); return 0; }, 1000);

		// 清理 for next fill
		hitChecks->hits->Clear();
	}
	else {
		// 直接做后续 Update
		UpdateEnd();
		calcResult = 0;
	}
#endif
	return 0;
};

#ifndef CC_TARGET_PLATFORM
inline void PKG::CatchFish::Scene::UpdateCalc(xx::Object_s && msg) noexcept {
	// 超时检查
	if (!msg) {
		xx::CoutTN("recv timeout.");
		calcResult = -1;
		return;
	}

	switch (msg->GetTypeId()) {
	case xx::TypeId_v<PKG::Calc_CatchFish::HitCheckResult>: {
		Handle(xx::As<PKG::Calc_CatchFish::HitCheckResult>(msg));
		calcResult = 0;
		break;
	}
	case xx::TypeId_v<PKG::Generic::Error>: {
		xx::CoutTN("recv error: ", msg);
		calcResult = -2;
		break;
	}
	default:
		xx::CoutTN("recv unhandled msg: ", msg);
		calcResult = -3;
		break;
	}
}

inline void PKG::CatchFish::Scene::Handle(PKG::Calc_CatchFish::HitCheckResult_s && msg) noexcept {
	// 整理合并
	for (auto&& b : *msg->bullets) {
		auto key = std::make_tuple(b.playerId, b.cannonId, b.bulletId);
		auto&& v = bullets[key];
		assert(v.bullet && v.bullet->coin == b.bulletCoin);
		v.count += b.bulletCount;
	}
	for (auto&& f : *msg->fishs) {
		auto key = std::make_tuple(f.playerId, f.cannonId, f.bulletId);
		auto&& v = bullets[key];
		v.fishIds.Add(f.fishId);
	}

	// 依次处理
	for (auto&& b : bullets) {
		// 结算( 会根据参数定位到 fish 并 Die, 会 refund )
		b.value.bullet->Bill(b.value);
	}

	// cleanup
	bullets.Clear();

	UpdateEnd();
}

inline void PKG::CatchFish::Scene::UpdateEnd() noexcept {
	// 存帧序号
	frameEvents->frameNumber = frameNumber;

	// todo: 要进一步减少重复序列化 scene, 就得修改完整同步流程, 将 self + token 分离发送

	// 预构造完整同步，减少冗余	
	PKG::CatchFish_Client::EnterSuccess_s enterSuccess;
	if (frameEnters.len) {
		xx::MakeTo(enterSuccess);
		xx::MakeTo(enterSuccess->players);
		for (auto&& plr_w : *players) {
			enterSuccess->players->Add(plr_w.lock());
		}
		enterSuccess->scene = catchFish->scene;		// todo: 如果是多个服务器实例则需要用 key 定位
		// 这两个不填充
		//enterSuccess->self
		//enterSuccess->token
	}

	// 将本帧事件推送给玩家
	for (auto&& plr_w : *players) {
		auto&& plr = xx::As<Player>(plr_w.lock());
		// 只给没断线的发
		if (plr->peer && !plr->peer->Disposed()) {
			// 如果是本帧内进入的玩家, 就下发完整同步
			if (frameEnters.Find(&*plr) != -1) {
				// 继续填充私有部分
				enterSuccess->self = plr_w;
				xx::MakeTo(enterSuccess->token, plr->token);
				// 发送
				plr->peer->SendPush(enterSuccess);
				plr->peer->Flush();
			}
			// 老玩家直接下发帧事件同步数据
			else {
				// 如果有数据就立即下发, 没有就慢发
				if (frameEvents->events->len || !(frameNumber & 0xF)) {
					plr->peer->SendPush(frameEvents);
					plr->peer->Flush();
				}
			}
		}

		// 临时输出一下 players 的 coin
		if (frameNumber % 120 == 0) {
			xx::Cout(" sit: ", plr->sit, " coin: ", plr->coin);
		}
	}
	if (frameNumber % 120 == 0 && players->len) {
		xx::CoutN();
	}

	frameEvents->events->Clear();		// 清除发送过的数据
	frameEnters.Clear();				// 清除发送过的数据
}

#endif
