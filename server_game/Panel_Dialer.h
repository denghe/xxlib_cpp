struct Dialer;
struct Panel_Dialer {
	// 指向 dialer( 由 dialer 填充 )
	Dialer* dialer = nullptr;
	Panel_Dialer(Dialer* dialer);

	RefHolder<cocos2d::Label> btnRedial;
	RefHolder<cocos2d::EventListenerTouchOneByOne> listenerRedial;

	RefHolder<cocos2d::Label> btnAutoFire;
	RefHolder<cocos2d::EventListenerTouchOneByOne> listenerAutoFire;

	RefHolder<cocos2d::Label> labelNumDialTimes;
	RefHolder<cocos2d::Label> labelPing;
	RefHolder<cocos2d::Label> labelNumFishs;
	RefHolder<cocos2d::Label> labelServer;

	void SetText_AutoFire(bool const& value) noexcept;
	void SetText_NumDialTimes(int64_t const& value) noexcept;
	void SetText_Ping(int64_t const& value) noexcept;
	void SetText_NumFishs(size_t const& value) noexcept;
	void SetText_Server(std::string const& value) noexcept;
};
using Panel_Dialer_s = std::shared_ptr<Panel_Dialer>;
