struct Dialer;
struct ClientPeer : xx::UvKcpPeer {
	using BaseType = xx::UvKcpPeer;
	using BaseType::BaseType;

	// 处理推送( 向 dialer.recvs 压入数据 )
	virtual int ReceivePush(xx::Object_s&& msg) noexcept override;
};
using ClientPeer_s = std::shared_ptr<ClientPeer>();
using ClientPeer_w = std::weak_ptr<ClientPeer>();
