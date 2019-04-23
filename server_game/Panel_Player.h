struct Dialer;
struct Panel_Player {
	// 指向 player( 由 player 填充 )
	Player* player = nullptr;
	Panel_Player(Player* player);

	int64_t lastCoin = -1;
	RefHolder<cocos2d::Label> labelCoin;
	void SetText_Coin(int64_t const& value) noexcept;
};
