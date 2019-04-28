struct Service;
struct PeerContext {
	// 一些便于调用的存储
	xx::IUvPeer_s peer;
	Service* service = nullptr;
	CatchFish* catchFish = nullptr;		// 所在游戏实例( 这个理论上应该从玩家来获取 )

	PeerContext(xx::IUvPeer_s& peer, Service* service);

	// 与 player 解绑( 被原始 peer Dispose 时调用 )
	~PeerContext();

	// 得到 peer 的 PeerContext
	static PeerContext& From(xx::IUvPeer_s const& peer);

	// Enter 成功后绑定到玩家
	PKG::CatchFish::Player_w player_w;

	// 首包判断 flag
	bool isFirstPackage = true;

	// 预创建 反复用
	inline static PKG::Generic::Pong_s pkgPong = xx::Make<PKG::Generic::Pong>();

	// 处理推送( 被原始 peer 调用 )
	int ReceivePush(xx::Object_s&& msg) noexcept;

	// 处理请求( 当前主要针对 ping )( 被原始 peer 调用 )
	int ReceiveRequest(int const& serial, xx::Object_s&& msg) noexcept;
};
