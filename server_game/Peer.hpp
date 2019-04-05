int Peer::ReceivePush(xx::Object_s&& msg) noexcept {
	// �Ѱ�����
	if (auto && player = player_w.lock()) {
		switch (msg->GetTypeId()) {
		case xx::TypeId_v<PKG::Client_CatchFish::Shoot>:
			player->recvShoots.Add(xx::As<PKG::Client_CatchFish::Shoot>(msg));
			break;
		case xx::TypeId_v<PKG::Client_CatchFish::Hit>:
			player->recvHits.Add(xx::As<PKG::Client_CatchFish::Hit>(msg));
			break;
		default:
			player->peer.reset();
			return -1;
		}
	}
	// ��������. ֻ���� Enter
	else {
		switch (msg->GetTypeId()) {
		case xx::TypeId_v<PKG::Client_CatchFish::Enter>: {
			// ���õ��������÷���ʹ��
			auto&& cfg = *catchFish->cfg;

			// �ж�Ҫ�����ĸ� scene (��ǰ��һ��, �� )
			auto&& scene = *catchFish->scene;

			// ������û��λ��. ���û�о�ֱ�ӶϿ�
			PKG::CatchFish::Sits sit;
			if (!scene.freeSits->TryPop(sit)) return -1;

			// �������������( ģ���Ѵ�db���������� )
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
			player->peer = xx::As<Peer>(this->shared_from_this());
			player->scene = &scene;
			player->sit = sit;
			xx::MakeTo(player->weapons);

			// ������ʼ��̨
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
				cannon->indexAtContainer = (int)player->cannons->len;
				cannon->player = &*player;
				cannon->pos = cfg.sitPositons->At((int)sit);
				cannon->quantity = cannonCfg->quantity;
				cannon->scene = &scene;
				cannon->shootCD = 0;
				player->cannons->Add(cannon);
			}
					// todo: more cannon types here
			default:
				return -2;
			}

			// ��������ͬ�����·�
			// todo

			break;
		}
		default:
			return -1;
		}
	}
	return 0;
}