struct Peer : xx::UvKcpPeer {
	using BaseType = xx::UvKcpPeer;
	using BaseType::BaseType;

	// 所在游戏实例( Listener Accept 时填充 )
	CatchFish* catchFish = nullptr;

	// Enter 成功后绑定到玩家
	Player_w player_w;

	// pong package cache for send
	inline static PKG::Generic::Pong_s pkgPong = xx::Make<PKG::Generic::Pong>();

	// 处理推送
	virtual int ReceivePush(xx::Object_s&& msg) noexcept override;

	virtual int ReceiveRequest(int const& serial, xx::Object_s&& msg) noexcept override;

	virtual void Dispose(int const& flag = 1) noexcept override;
};
