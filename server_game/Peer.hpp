inline void Peer::Dispose(int const& flag) noexcept {
	if (this->Disposed()) return;
	// todo: kick player?
	this->BaseType::Dispose(flag);
}

inline int Peer::ReceivePush(xx::Object_s&& msg) noexcept {

	// todo: 简单状态以识别是否为首包. 启用超时机制

	if (auto && player = player_w.lock()) {
		// 已绑定连接
		// 将初步判定合法的消息放入分类容器, 待到适当时机读出使用, 模拟输入
		switch (msg->GetTypeId()) {
		case xx::TypeId_v<PKG::Client_CatchFish::Fire>:
			player->recvFires.push_back(xx::As<PKG::Client_CatchFish::Fire>(msg));
			if (player->recvFires.size() > 200) return -1;			// 简单包堆积检测
			break;
		case xx::TypeId_v<PKG::Client_CatchFish::Hit>:
			player->recvHits.push_back(xx::As<PKG::Client_CatchFish::Hit>(msg));
			if (player->recvHits.size() > 200) return -1;			// 简单包堆积检测
			break;
		default:
			player->peer.reset();
			return -1;
		}
	}
	else {
		// 匿名连接. 只接受 Enter
		switch (msg->GetTypeId()) {
		case xx::TypeId_v<PKG::Client_CatchFish::Enter>: {
			// 引用到公共配置方便使用
			auto&& cfg = *catchFish->cfg;

			// 判断要进入哪个 scene (当前就一个, 略 )
			auto&& scene = *catchFish->scene;

			// 看看有没有位置. 如果没有就直接断开
			PKG::CatchFish::Sits sit;
			if (!scene.freeSits->TryPop(sit)) return -1;

			// 构建玩家上下文( 模拟已从db读到了数据 )
			auto&& player = xx::Make<Player>();
			player->autoFire = false;
			player->autoIncId = 0;
			player->autoLock = false;
			player->avatar_id = 0;
			xx::MakeTo(player->cannons);
			player->coin = 1000;
			player->consumeCoin = 0;
			player->id = (int)sit;
			xx::MakeTo(player->nickname, "player_");
			player->nickname->append(std::to_string((int)sit));
			player->noMoney = false;
			player->scene = &scene;
			player->sit = sit;
			xx::MakeTo(player->weapons);

			// 构建初始炮台
			auto&& cannonCfgId = 0;
			switch (cannonCfgId) {
			case 0: {
				auto&& cannonCfg = cfg.cannons->At(cannonCfgId);
				auto&& cannon = xx::Make<Cannon>();
				cannon->angle = float(cannonCfg->angle);
				xx::MakeTo(cannon->bullets);
				cannon->cfg = &*cannonCfg;
				cannon->cfgId = cannonCfgId;
				cannon->coin = 1;
				cannon->id = (int)player->cannons->len;
				cannon->player = &*player;
				cannon->pos = cfg.sitPositons->At((int)sit);
				cannon->quantity = cannonCfg->quantity;
				cannon->scene = &scene;
				cannon->fireCD = 0;
				player->cannons->Add(cannon);
				break;
			}
			// todo: more cannon types here
			default:
				return -2;
			}

			// 将玩家放入相应容器
			catchFish->players.Add(player);
			scene.players->Add(player);
			scene.frameEnters.Add(&*player);

			// 玩家与连接绑定
			player_w = player;
			player->peer = xx::As<Peer>(this->shared_from_this());

			// 构建玩家进入通知并放入帧同步下发事件集合待发
			auto&& enter = xx::Make<PKG::CatchFish::Events::Enter>();
			enter->avatar_id = player->avatar_id;
			enter->cannonCfgId = player->cannons->At(0)->cfgId;
			enter->cannonCoin = player->cannons->At(0)->coin;
			enter->coin = player->coin;
			enter->nickname = player->nickname;
			enter->noMoney = player->noMoney;
			enter->playerId = player->id;
			enter->sit = player->sit;
			scene.frameEvents->events->Add(enter);
			break;
		}
		default:
			return -1;
		}
	}
	return 0;
}
