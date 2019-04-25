struct Listener;
struct Peer : xx::UvKcpPeer {
	using BaseType = xx::UvKcpPeer;
	using BaseType::BaseType;

	// 引用到 listener( Listener Accept 时填充 )
	Listener* listener = nullptr;

	// 所在游戏实例( Listener Accept 时填充 )
	CatchFish* catchFish = nullptr;

	// Enter 成功后绑定到玩家
	PKG::CatchFish::Player_w player_w;

	// 首包判断 flag
	bool isFirstPackage = true;

	// 预创建 反复用
	inline static PKG::Generic::Pong_s pkgPong = xx::Make<PKG::Generic::Pong>();

	// 处理推送
	virtual int ReceivePush(xx::Object_s&& msg) noexcept override;

	// 处理请求( 当前主要针对 ping )
	virtual int ReceiveRequest(int const& serial, xx::Object_s&& msg) noexcept override;

	// 附带与 player 解绑 的功能
	virtual void Dispose(int const& flag = 1) noexcept override;
};
