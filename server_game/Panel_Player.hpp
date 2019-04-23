inline Panel_Player::Panel_Player(Player* player)
	: player(player) {
	{
		labelCoin = cocos2d::Label::createWithSystemFont("", "", 32);
		labelCoin->setPosition(player->pos + xx::Pos{0, 30});
		labelCoin->setAnchorPoint({ 0.5, 1 });
		labelCoin->setGlobalZOrder(500);
		cc_scene->addChild(labelCoin);
		SetText_Coin(player->coin);						// 更新显示内容
	}
}

inline void Panel_Player::SetText_Coin(int64_t const& value) noexcept {
	assert(!::catchFish->disposed);
	if (!labelCoin) return;
	if (!value == lastCoin) return;
	labelCoin->setString(std::to_string(value));
	lastCoin = value;
}
