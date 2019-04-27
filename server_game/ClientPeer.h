struct Dialer;
#if USE_UDP_KCP
struct ClientPeer : xx::UvKcpPeer {
	using BaseType = xx::UvKcpPeer;
#else
struct ClientPeer : xx::UvTcpPeer {
	using BaseType = xx::UvTcpPeer;
#endif
	using BaseType::BaseType;

	// 处理推送( 向 dialer.recvs 压入数据 )
	virtual int ReceivePush(xx::Object_s&& msg) noexcept override;
};
using ClientPeer_s = std::shared_ptr<ClientPeer>();
using ClientPeer_w = std::weak_ptr<ClientPeer>();
