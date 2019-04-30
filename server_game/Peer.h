struct Service;
struct Peer : xx::UvPeer {
	using xx::UvPeer::UvPeer;
	// 一些便于调用的存储
	Service* service = nullptr;
	CatchFish* catchFish = nullptr;		// 所在游戏实例( 这个理论上应该从玩家来获取 )

	// Enter 成功后绑定到玩家
	PKG::CatchFish::Player_w player_w;

	// 首包判断 flag
	bool isFirstPackage = true;

	// 预创建 反复用
	inline static PKG::Generic::Pong_s pkgPong = xx::Make<PKG::Generic::Pong>();

	// 处理推送( 被原始 peer 调用 )
	virtual int ReceivePush(xx::Object_s&& msg) noexcept override;

	// 处理请求( 当前主要针对 ping )( 被原始 peer 调用 )
	virtual int ReceiveRequest(int const& serial, xx::Object_s&& msg) noexcept override;
};
using Peer_s = std::shared_ptr<Peer>;
