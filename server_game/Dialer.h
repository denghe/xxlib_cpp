struct Dialer {
	Dialer();

	// 拨号器
	xx::UvDialer_s dialer;

	// 创建拨号器
	void MakeDialer();

	// 拨号后得到的连接
	xx::UvPeer_s peer;

	// 收到的数据
	std::deque<xx::Object_s> recvs;

	// 脚本行号
	int lineNumber = 0;

	// 脚本用变量
	bool finished = false;
	int r = 0;
	int64_t waitMS = 0;
	std::vector<std::string> ips;
	xx::UvResolver_s resolver;
	int timeoutFrameNumber = 0;
	int numDialTimes = 0;
	bool autoFire = false;
	int64_t ping = 0;
	xx::Random rnd;
	std::string token;							// 保存当前玩家 token
	PKG::CatchFish::Player_s player;			// 指向当前玩家


	// 处理首包( EnterSuccess || Error )
	int HandleFirstPackage() noexcept;

	// 处理一般数据包( 总路由 )
	int HandlePackagesOrUpdateScene() noexcept;

	// 处理事件包
	int HandleEvents(PKG::CatchFish::Events::Event_s const& e) noexcept;

	// 分别处理事件包
	int Handle(PKG::CatchFish::Events::Enter_s o) noexcept;
	int Handle(PKG::CatchFish::Events::Leave_s o) noexcept;
	int Handle(PKG::CatchFish::Events::NoMoney_s o) noexcept;
	int Handle(PKG::CatchFish::Events::Refund_s o) noexcept;
	int Handle(PKG::CatchFish::Events::FishDead_s o) noexcept;
	int Handle(PKG::CatchFish::Events::PushWeapon_s o) noexcept;
	int Handle(PKG::CatchFish::Events::PushFish_s o) noexcept;
	int Handle(PKG::CatchFish::Events::OpenAutoLock_s o) noexcept;
	int Handle(PKG::CatchFish::Events::Aim_s o) noexcept;
	int Handle(PKG::CatchFish::Events::CloseAutoLock_s o) noexcept;
	int Handle(PKG::CatchFish::Events::OpenAutoFire_s o) noexcept;
	int Handle(PKG::CatchFish::Events::CloseAutoFire_s o) noexcept;
	int Handle(PKG::CatchFish::Events::Fire_s o) noexcept;
	int Handle(PKG::CatchFish::Events::CannonSwitch_s o) noexcept;
	int Handle(PKG::CatchFish::Events::CannonCoinChange_s o) noexcept;


	// 清空 recvs, player, catchFish->players, scene
	void Reset() noexcept;

	// 脚本实现
	int UpdateCore(int const& lineNumber) noexcept;

	// 每帧驱动脚本
	int Update() noexcept;

	// packages cache
	inline static PKG::Client_CatchFish::Enter_s pkgEnter = xx::Make<PKG::Client_CatchFish::Enter>();
	inline static PKG::Generic::Ping_s pkgPing = xx::Make<PKG::Generic::Ping>();



	// 面板显示相关
	RefHolder<cocos2d::Label> btnSwitchTcpKcp;
	RefHolder<cocos2d::EventListenerTouchOneByOne> listenerSwitchTcpKcp;

	RefHolder<cocos2d::Label> btnRedial;
	RefHolder<cocos2d::EventListenerTouchOneByOne> listenerRedial;

	RefHolder<cocos2d::Label> btnAutoFire;
	RefHolder<cocos2d::EventListenerTouchOneByOne> listenerAutoFire;

	RefHolder<cocos2d::Label> labelNumDialTimes;
	RefHolder<cocos2d::Label> labelPing;
	RefHolder<cocos2d::Label> labelNumFishs;
	RefHolder<cocos2d::Label> labelServer;

	void SetText_TcpKcp(bool const& value) noexcept;
	void SetText_AutoFire(bool const& value) noexcept;
	void SetText_NumDialTimes(int64_t const& value) noexcept;
	void SetText_Ping(int64_t const& value) noexcept;
	void SetText_NumFishs(size_t const& value) noexcept;
	void SetText_Server(std::string const& value) noexcept;

	void DrawInit() noexcept;
};
using Dialer_s = std::shared_ptr<Dialer>;
