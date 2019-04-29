inline PeerContext::PeerContext(xx::IUvPeer_s& peer, Service* service)
	: peer(peer)
	, service(service)
	, catchFish(&*service->catchFish) {
	peer->userData = this;
	peer->OnDisconnect([peer] {
		assert(peer->userData);
		delete (PeerContext*)peer->userData;
		peer->userData = nullptr;
	});
	peer->OnReceivePush([this](xx::Object_s && msg) { return this->ReceivePush(std::move(msg)); });
	peer->OnReceiveRequest([this](int const& serial, xx::Object_s && msg) { return this->ReceiveRequest(serial, std::move(msg)); });
}

inline PeerContext::~PeerContext() noexcept {
	// 解绑
	if (auto&& p = player_w.lock()) {
		p->peer.reset();
	}
}

PeerContext& PeerContext::From(xx::IUvPeer_s const& peer) {
	return *(PeerContext*)peer->userData;
}

inline int PeerContext::ReceiveRequest(int const& serial, xx::Object_s&& msg) noexcept {
	switch (msg->GetTypeId()) {
	case xx::TypeId_v<PKG::Generic::Ping>: {
		// 重置超时
		if (auto && p = player_w.lock()) {
			p->ResetTimeoutFrameNumber();
		}
		else {
			peer->ResetTimeoutMS(10000);
		}

		// 携带收到的数据回发
		pkgPong->ticks = xx::As<PKG::Generic::Ping>(msg)->ticks;
		auto r = peer->SendResponse(serial, pkgPong);
		peer->Flush();
		return r;
	}
	default:
		xx::CoutTN("recv unhandled request: ", msg);
		return -1;
	}
}

inline int PeerContext::ReceivePush(xx::Object_s&& msg) noexcept {

	// 有玩家 bind
	if (auto && player = player_w.lock()) {
		// 已绑定连接
		// 将初步判定合法的消息放入分类容器, 待到适当时机读出使用, 模拟输入
		switch (msg->GetTypeId()) {
		case xx::TypeId_v<PKG::Client_CatchFish::Fire>:
			player->recvFires.push_back(xx::As<PKG::Client_CatchFish::Fire>(msg));
			break;
		case xx::TypeId_v<PKG::Client_CatchFish::Hit>:
			player->recvHits.push_back(xx::As<PKG::Client_CatchFish::Hit>(msg));
			break;
		default:
			xx::CoutTN("binded recv unhandled push: ", msg);
			return -1;
		}
	}
	// 没玩家 bind
	else {
		if (!isFirstPackage) return -1;
		isFirstPackage = false;

		// 匿名连接. 只接受 Enter
		switch (msg->GetTypeId()) {
		case xx::TypeId_v<PKG::Client_CatchFish::Enter>: {
			auto&& o = xx::As<PKG::Client_CatchFish::Enter>(msg);

			// 引用到公共配置方便使用
			auto&& cfg = *catchFish->cfg;

			// 判断要进入哪个 scene (当前就一个, 略 )
			auto&& scene = *catchFish->scene;

			// 如果有传入玩家 id, 就试着定位, 走断线重连逻辑
			while (o->token) {
				// 用 token 查找玩家
				for(auto&& p : catchFish->players) {
					if (p->token == *o->token) {
						assert(p->peer != peer);
						// 踢掉原有连接( 可能性: 客户端很久没收到推送, 自己 redial, 而 server 并没发现断线 )
						p->Kick(peer->GetIP(), " reconnect");
						// 玩家与连接绑定
						player_w = p;
						p->peer = peer;
						// 放入本帧进入游戏的列表, 以便下发完整同步
						scene.frameEnters.Add(&*p);
						// 设置超时
						p->ResetTimeoutFrameNumber();
						// 返回成功
						return 0;
					}
				}
				break;
			}

			// 看看有没有位置. 如果没有就直接断开
			PKG::CatchFish::Sits sit;
			if (!scene.freeSits->TryPop(sit)) {
				xx::CoutTN("no more free sit: ", msg);
				return -2;
			}

			// 构建玩家上下文( 模拟已从db读到了数据 )
			auto&& player = xx::Make<PKG::CatchFish::Player>();
			xx::MakeTo(player->cannons);
			xx::MakeTo(player->events);
			player->scene = &scene;
			player->id = ++::service->playerAutoId;
			xx::Append(player->token, xx::Guid(true));
			player->sit = sit;
			player->pos = scene.cfg->sitPositons->At((int)sit);
			player->coin = 100000;
			xx::MakeTo(player->nickname, "player_" + std::to_string(player->id));
			player->autoFire = false;
			player->autoIncId = 0;
			player->autoLock = false;
			player->avatar_id = 0;
			player->noMoney = false;
			xx::MakeTo(player->weapons);

			// 构建初始炮台
			auto&& cannonCfgId = 0;
			switch (cannonCfgId) {
			case 0: {
				auto&& cannonCfg = cfg.cannons->At(cannonCfgId);
				auto&& cannon = xx::Make<PKG::CatchFish::Cannon>();
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
				xx::CoutTN("unbind recv unhandled cannon cfg id: ", msg);
				return -3;
			}

			// 将玩家放入相应容器
			catchFish->players.Add(player);
			scene.players->Add(player);
			scene.frameEnters.Add(&*player);

			// 玩家与连接绑定
			player_w = player;
			player->peer = peer;

			// 构建玩家进入通知并放入帧同步下发事件集合待发
			{
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
			}

			// 设置超时
			player->ResetTimeoutFrameNumber();

			// 成功退出
			xx::CoutTN(peer->GetIP(), " player enter. ", player);
			break;
		}
		default:
			xx::CoutTN("unbind recv unhandled push: ", msg);
			return -4;
		}
	}
	return 0;
}
