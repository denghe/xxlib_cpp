//#include <xx__http.h>
#include <xx_epoll2.hpp>
#include <unordered_set>
namespace EP = xx::Epoll;

#define PRINT_LOG_SERVICE_CONFIG			1
#define PRINT_LOG_SERVICE0_NOT_READY		0
#define PRINT_LOG_SERVICE_CONNECTD			1
#define PRINT_LOG_SERVICE_DISCONNECTD		1
#define PRINT_LOG_SERVICE_DIAL				0

#define PRINT_LOG_CLIENT_PEER_ACCEPT		1
#define PRINT_LOG_CLIENT_PEER_DISCONNECT	1

#define PRINT_LOG_CP_SENDTO_SP				0
#define PRINT_LOG_SP_SENDTO_CP				0

#define PRINT_LOG_RECV_OPEN					1
#define PRINT_LOG_RECV_CLOSE				1
#define PRINT_LOG_RECV_KICK					1



/***********************************************************************************************************************/
// �����ļ����
/***********************************************************************************************************************/

#include"ajson.hpp"

struct ServiceInfo {
	int serviceId = 0;
	std::string ip;
	int port = 0;
};
AJSON(ServiceInfo, serviceId, ip, port);

struct ServiceCfg {
	int gatewayId = 0;						// ��ǰ�����ڲ����
	std::string listenIP;					// �������ĸ� ip ��( ͨ��Ϊ 0.0.0.0 )
	int listenPort = 0;						// �����˿�
	int listenTcpKcpOpt = 0;				// Э��ѡ��: 0: tcp only;  1: kcp ony;   2: tcp + kcp
	int clientTimeoutMS = 0;				// �ͻ��˵��߼��ʱ�� ms. �������ʱ��û���յ��ͻ��˵ĺϷ��������ϵ��ͻ���
	std::string webListenIP;				// �������������ĸ� ip ��( ͨ��Ϊ 0.0.0.0 )
	int webListenPort = 0;					// �����˿�
	std::vector<ServiceInfo> services;		// Ҫ���ӵ���Щ����
};
AJSON(ServiceCfg, gatewayId, listenIP, listenPort, listenTcpKcpOpt, clientTimeoutMS, webListenIP, webListenPort, services);

//struct WebHandler;



/***********************************************************************************************************************/
// ͨ�� peer ���
/***********************************************************************************************************************/

// �ɼ̳��� TcpPeer �� KcpPeer. ��չ�߽׹���
template<typename Base>
struct Peer : Base {
	// �����ڲ�ָ���( 4 �ֽڳ��� + 0xFFFFFFFF + ����( ͨ��Ϊ cmd string + args... ) )
	std::function<void(uint8_t* const& buf, std::size_t const& len)> onReceiveCommand;

	// ����һ�����ݰ�( 4 �ֽڳ��� + 4�ֽڵ�ַ + ���� )
	std::function<void(uint8_t* const& buf, std::size_t const& len)> onReceivePackage;

	// ��������¼�
	std::function<void()> onDisconnect;

	// ��ʼ�� bb д��. �ճ� ���� ͷ��
	static void WritePackageBegin(xx::BBuffer& bb, size_t const& reserveLen, uint32_t const& addr) {
		bb.Reserve(4 + reserveLen);
		bb.len = 4;
		bb.WriteFixed(addr);
	}

	// ���� bb д�����������ݳ��� ��д ��ͷ
	static void WritePackageEnd(xx::BBuffer& bb) {
		*(uint32_t*)bb.buf = (uint32_t)(bb.len - 4);
	}

	// �����ڲ�ָ���. cmd string + args...
	template<typename...Args>
	void SendCommand(Args const& ... cmdAndArgs) {
		auto&& bb = this->ep->sendBB;
		WritePackageBegin(bb, 1024, 0xFFFFFFFFu);
		bb.Write(cmdAndArgs...);
		WritePackageEnd(bb);
		this->Send(bb);
	}

	virtual void OnReceive() override {
		// ȡ��ָ�뱸��
		auto buf = (uint8_t*)this->recv.buf;
		auto end = (uint8_t*)this->recv.buf + this->recv.len;

		// ȷ����ͷ���ȳ���
		while (buf + 4 <= end) {

			// ȡ������������ �� �жϺϷ���
			auto dataLen = *(uint32_t*)buf;
			if (dataLen > this->ep->maxPackageLength) {
				this->Dispose();
				return;
			}

			// ���������������˳�
			if (buf + dataLen > end) break;

			// ������������ʼ���ô���ص�
			buf += 4;
			{
				// �����жϱ���
				EP::Ref<Base> alive(this);

				// �ж��Ƿ�Ϊ�ڲ�ָ��
				if (*(uint32_t*)buf == 0xFFFFFFFFu) {
					onReceiveCommand(buf + 4, dataLen - 4);
				}
				else {
					onReceivePackage(buf + 4, dataLen - 4);
				}

				// �����ǰ��ʵ������ɱ���˳�
				if (!alive) return;
			}
			// ������һ�����Ŀ�ͷ
			buf += dataLen;
		}

		// �Ƴ����Ѵ��������
		this->recv.RemoveFront((char*)buf - this->recv.buf);
	}

	virtual void OnDisconnect(int const& reason) override {
		if (onDisconnect) {
			onDisconnect();
		}
	}
};

// �ɼ̳��� TcpPeer �� KcpPeer
template<typename Base>
struct FromClientPeer : Peer<Base> {
	// �������, accept ʱ���
	uint32_t clientId = 0xFFFFFFFFu;

	// ������ʵ� service peers �� id �İ�����
	std::unordered_set<uint32_t> serviceIds;

	void SendCommand_Open(uint32_t const& serviceId) {
		this->SendCommand("open", serviceId);
	}

	void SendCommand_Close(uint32_t const& serviceId) {
		this->SendCommand("close", serviceId);
	}
};

struct ToServicePeer : Peer<EP::TcpPeer> {
	// �ڲ�������, ���������
	uint32_t serviceId = 0xFFFFFFFFu;

	// �ȴ� ping �ذ���״ֵ̬
	bool waitPingBack = false;

	void SendCommand_GatewayId(uint32_t const& gatewayId) {
		this->SendCommand("gatewayId", gatewayId);
	}

	void SendCommand_Accept(uint32_t const& clientId, std::string const& ip) {
		this->SendCommand("accept", clientId, ip);
	}

	void SendCommand_Disconnect(uint32_t const& clientId) {
		this->SendCommand("disconnect", clientId);
	}
	void SendCommand_Ping() {
		this->SendCommand("ping", xx::NowEpoch10m());
	}
};

struct Gateway;

struct ToServiceDialer : EP::Dialer {
	// ��¼�����ӳɹ�ʱ Ҫӳ�䵽�� ���� id
	uint32_t serviceId = 0;

	virtual EP::Peer_u OnCreatePeer(bool const& isKcp) override {
		assert(!isKcp);
		return xx::TryMakeU<ToServicePeer>();
	}

	std::function<void(EP::Ref<ToServicePeer> const& peer)> onConnect;
	virtual void OnConnect(EP::Peer_r const& peer) override {
		onConnect(peer.As<ToServicePeer>());
	}
};

// �ɼ̳��� TcpListener �� KcpListener
template<typename BaseListener>
struct FromClientListener : BaseListener {
	// ָ�������������
	Gateway* gw = nullptr;

	virtual std::unique_ptr<typename BaseListener::PeerType> OnCreatePeer() override {
		return xx::TryMakeU<FromClientPeer<typename BaseListener::PeerType>>();
	}

	virtual void OnAccept(EP::Ref<typename BaseListener::PeerType> const& peer) override;
};




/***********************************************************************************************************************/
// ��������
/***********************************************************************************************************************/

struct Gateway {
	EP::Context ep;

	// ������������. �� json ����
	ServiceCfg cfg;

	/***********************************************************************************************/
	// constructor
	/***********************************************************************************************/
	Gateway() {
		ajson::load_from_file(cfg, "service_cfg.json");

		ep.CreateTimer(5 * 100, [this](auto t) {
			for (auto&& kv : serviceDialerPeers) {
				if (auto&& peer = kv.second.second.Lock()) {
					if (!peer->waitPingBack)
					{
						peer->waitPingBack = true;
						peer->SendCommand_Ping();
					}
					else
					{
						peer->Dispose();
					}
				}
			}
			t->SetTimeout(5 * 100);
			});
		InitClientListener();
		InitServiceDialers();
		//InitWebListener();
	}


	/***********************************************************************************************/
	// client
	/***********************************************************************************************/

	// �ȴ� client �Ľ���
	EP::Ref<FromClientListener<EP::TcpListener>> clientTcpListener;
	EP::Ref<FromClientListener<EP::KcpListener>> clientKcpListener;

	// clientPeerAutoId += 2;  new cp.clientId = clientPeerAutoId + kcp ? 1 : 0
	uint32_t clientPeerAutoId = 0;

	// key: clientId. kcp client id Ϊ����
	std::unordered_map<uint32_t, EP::Ref<FromClientPeer<EP::TcpPeer>>> clientTcpPeers;
	std::unordered_map<uint32_t, EP::Ref<FromClientPeer<EP::KcpPeer>>> clientKcpPeers;

	void InitClientListener() {
		// ���� listener
		clientTcpListener = ep.CreateTcpListener<FromClientListener<EP::TcpListener>>(cfg.listenPort);
		clientKcpListener = ep.CreateUdpPeer<FromClientListener<EP::KcpListener>>(cfg.listenPort);

		clientTcpListener->gw = this;
		clientKcpListener->gw = this;
	}



	/***********************************************************************************************/
	// service
	/***********************************************************************************************/

	// �������� peer �洢��һ�𣬰󶨹�ϵ
	using DialerPeer = std::pair<EP::Ref<ToServiceDialer>, EP::Ref<ToServicePeer>>;

	// key: serviceId
	std::unordered_map<uint32_t, DialerPeer> serviceDialerPeers;


	void InitServiceDialers() {
		// ���������� timer
		ep.CreateTimer(50, [this](auto t) {
			for (auto&& kv : serviceDialerPeers) {
				//auto&& serviceId = kv.first;
				auto&& dialer = kv.second.first;
				if (!kv.second.second) {
					if (!dialer->Busy()) {
						dialer->Dial(200);
#if PRINT_LOG_SERVICE_DIAL
						xx::CoutN("service dialer dial...");
#endif
					}
				}
			}
			t->SetTimeout(50);
			});

		// ���� dialers
		for (auto&& cfg : cfg.services) {
			TryCreateServiceDialer(cfg.serviceId, cfg.ip, cfg.port);
#if PRINT_LOG_SERVICE_CONFIG
			xx::CoutN("serviceId:", cfg.serviceId, "  ip:", cfg.ip, "  port:", cfg.port);
#endif
		}
	}

	int TryCreateServiceDialer(uint32_t const& serviceId, std::string const& ip, int const& port) {
		auto&& dialer = serviceDialerPeers[serviceId].first;
		if (dialer) return -1;

		dialer = ep.CreateDialer<ToServiceDialer>();
		if (!dialer) return -2;

		dialer->serviceId = serviceId;
		dialer->AddAddress(ip, port);

		// ������Ӧ�¼��������
		dialer->onConnect = [this, serviceId](auto peer) {
			// ���û����, ����
			auto&& sp = peer.Lock();
			if (!sp) return;

			// ������ serviceId
			sp->serviceId = serviceId;

			// ��������
			serviceDialerPeers[serviceId].second = sp;

			// ע���¼����Ͽ�ʱ�����Ӧ peer �洢����
			sp->onDisconnect = [this, sp] {

				// �Ӵ洢���Ƴ�
				this->serviceDialerPeers[sp->serviceId].second.Reset();

				// ������ client peers ��İ��������Ƴ� ���Զ��·� close.	// todo: ���  ������ ���ˣ�ֱ������Ͽ�
				for (auto&& kv : clientTcpPeers) {
					if (auto&& c = kv.second.Lock()) {
						c->SendCommand_Close(sp->serviceId);
					}
				}
				for (auto&& kv : clientKcpPeers) {
					if (auto&& c = kv.second.Lock()) {
						c->SendCommand_Close(sp->serviceId);
					}
				}

#if PRINT_LOG_SERVICE_DISCONNECTD
				xx::CoutN("service peer disconnect: ", sp->addr, ", serviceId = ", sp->serviceId);
#endif
			};

			// ע���¼����յ����͵Ĵ���
			sp->onReceivePackage = [this, sp](uint8_t* const& buf, std::size_t const& len) {
				// ���� clientId
				uint32_t clientId = 0;
				if (len < sizeof(clientId)) {
					sp->Dispose();
					return;
				}
				clientId = *(uint32_t*)buf;

#if PRINT_LOG_SP_SENDTO_CP
				xx::CoutN("sp -> cp. size = ", len, ", clientId = ", clientId);
#endif
				// kcp
				if (clientId & 1) {
					// ���û�ҵ� ���ѶϿ� �򷵻أ����Դ���
					auto&& iter = this->clientKcpPeers.find(clientId);
					if (iter == this->clientKcpPeers.end()) return;
					auto&& cp = iter->second.Lock();
					if (!cp) return;

					// �۸� clientId Ϊ serviceId ת��( �� header )
					*(uint32_t*)buf = sp->serviceId;
					cp->Send((char*)buf - 4, len + 4);
				}
				else {
					// ���û�ҵ� ���ѶϿ� �򷵻أ����Դ���
					auto&& iter = this->clientTcpPeers.find(clientId);
					if (iter == this->clientTcpPeers.end()) return;
					auto&& cp = iter->second.Lock();
					if (!cp) return;

					// �۸� clientId Ϊ serviceId ת��( �� header )
					*(uint32_t*)buf = sp->serviceId;
					cp->Send((char*)buf - 4, len + 4);
				}
			};

			// ע���¼����յ��ڲ�ָ��Ĵ���
			sp->onReceiveCommand = [this, sp](uint8_t* const& buf, std::size_t const& len) {
				// ��� bb ���������
				auto&& bb = ep.recvBB;
				bb.Reset(buf, len);

				// �Զ�ȡ cmd �ִ�
				std::string cmd;
				if (int r = bb.Read(cmd)) {
					sp->Dispose();
					return;
				}

				// ���˿�. ����: clientId
				if (cmd == "open") {
					// �Զ��� clientId
					uint32_t clientId = 0;
					if (int r = bb.Read(clientId)) {
						sp->Dispose();
						return;
					}

					// kcp
					if (clientId & 1) {
						// ���û�ҵ� ���ѶϿ� �򷵻أ����Դ���
						auto&& iter = this->clientKcpPeers.find(clientId);
						if (iter == this->clientKcpPeers.end()) return;
						auto&& cp = iter->second.Lock();
						if (!cp) return;

						// ���������
						cp->serviceIds.emplace(sp->serviceId);

						// �·� open
						cp->SendCommand_Open(sp->serviceId);
					}
					else {
						// ���û�ҵ� ���ѶϿ� �򷵻أ����Դ���
						auto&& iter = this->clientTcpPeers.find(clientId);
						if (iter == this->clientTcpPeers.end()) return;
						auto&& cp = iter->second.Lock();
						if (!cp) return;

						// ���������
						cp->serviceIds.emplace(sp->serviceId);

						// �·� open
						cp->SendCommand_Open(sp->serviceId);
					}

#if PRINT_LOG_RECV_OPEN
					xx::CoutN("service peer recv cmd open: clientId: ", clientId, ", serviceId = ", sp->serviceId);
#endif
				}


				// �ض˿�. ����: clientId
				else if (cmd == "close") {
					// �Զ��� clientId
					uint32_t clientId = 0;
					if (int r = bb.Read(clientId)) {
						sp->Dispose();
						return;
					}

					// ǰ�ü��
					if (!clientId) {
						sp->Dispose();
						return;
					}

					// kcp
					if (clientId & 1) {
						// ���û�ҵ� ���ѶϿ� �򷵻أ����Դ���
						auto&& iter = this->clientKcpPeers.find(clientId);
						if (iter == this->clientKcpPeers.end()) return;
						auto&& cp = iter->second.Lock();
						if (!cp) return;

						// �Ӱ������Ƴ�
						cp->serviceIds.erase(sp->serviceId);

						// �·� close
						cp->SendCommand_Close(sp->serviceId);
					}
					else {
						// ���û�ҵ� ���ѶϿ� �򷵻أ����Դ���
						auto&& iter = this->clientTcpPeers.find(clientId);
						if (iter == this->clientTcpPeers.end()) return;
						auto&& cp = iter->second.Lock();
						if (!cp) return;

						// �Ӱ������Ƴ�
						cp->serviceIds.erase(sp->serviceId);

						// �·� close
						cp->SendCommand_Close(sp->serviceId);
					}

#if PRINT_LOG_RECV_CLOSE
					xx::CoutN("service peer recv cmd close: clientId: ", clientId, ", serviceId = ", sp->serviceId);
#endif
				}
				else if (cmd == "ping") {
					sp->waitPingBack = false;
					return;
				}
				// ���������. ����: clientId, delayMS
				else if (cmd == "kick") {
					// �Զ�������
					uint32_t clientId = 0;
					int64_t delayMS = 0;
					if (int r = bb.Read(clientId, delayMS)) {
						sp->Dispose();
						return;
					}

					// ǰ�ü��
					if (!clientId) {
						sp->Dispose();
						return;
					}

					// kcp
					if (clientId & 1) {
						// ���û�ҵ� ���ѶϿ� �򷵻أ����Դ���
						auto&& iter = this->clientKcpPeers.find(clientId);
						if (iter == this->clientKcpPeers.end()) return;
						auto&& cp = iter->second.Lock();
						if (!cp) return;

						if (delayMS) {
							// ׷��һ�� close ָ���Ա� client �յ���ֱ����ɱ
							cp->SendCommand_Close(0);

							// �ӳٶϿ����Ƚ���¼��������������ó�ʱʱ������ʱ�� Dispose()
							auto cp1 = cp;							// onDisconnect �ᵼ�� cp ����ʧЧ�ʸ���
							cp1->onDisconnect();					// ���ӳ�䲢���Ͷ���֪ͨ
							cp1->onDisconnect = [cp1] {};			// ����ɺ���������
							cp1->onReceivePackage = nullptr;
							cp1->SetTimeout((int)delayMS / 10);
						}
						else {
							// ���̶Ͽ����ӣ����� onDisconnect( �� this->clientPeers �Ƴ���������� serviceIds ��Ӧ peer �㲥�Ͽ�֪ͨ )
							cp->Dispose();
						}
					}
					else {
						// ���û�ҵ� ���ѶϿ� �򷵻أ����Դ���
						auto&& iter = this->clientTcpPeers.find(clientId);
						if (iter == this->clientTcpPeers.end()) return;
						auto&& cp = iter->second.Lock();
						if (!cp) return;

						if (delayMS) {
							// ׷��һ�� close ָ���Ա� client �յ���ֱ����ɱ
							cp->SendCommand_Close(0);

							// �ӳٶϿ����Ƚ���¼��������������ó�ʱʱ������ʱ�� Dispose()
							auto cp1 = cp;							// onDisconnect �ᵼ�� cp ����ʧЧ�ʸ���
							cp1->onDisconnect();					// ���ӳ�䲢���Ͷ���֪ͨ
							cp1->onDisconnect = [cp1] {};			// ����ɺ���������
							cp1->onReceivePackage = nullptr;
							cp1->SetTimeout((int)delayMS / 10);
						}
						else {
							// ���̶Ͽ����ӣ����� onDisconnect( �� this->clientPeers �Ƴ���������� serviceIds ��Ӧ peer �㲥�Ͽ�֪ͨ )
							cp->Dispose();
						}
					}

#if PRINT_LOG_RECV_KICK
					xx::CoutN("service peer recv cmd kick: clientId: ", clientId);
#endif
				}

				else {
					sp->Dispose();
					return;
				}
			};

			// �� service �����Լ��� gatewayId
			sp->SendCommand_GatewayId(cfg.gatewayId);

#if PRINT_LOG_SERVICE_CONNECTD
			xx::CoutN("service peer connected to: ", sp->addr, ", serviceId = ", sp->serviceId);
#endif
		};

		return dialer->Dial(200);
	}

	int RemoveServiceDialerPeer(uint32_t const& serviceId) {
		auto&& iter = serviceDialerPeers.find(serviceId);
		if (iter == serviceDialerPeers.end())
			return -1;

		auto&& dialer = iter->second.first;
		auto&& peer = iter->second.second;

		if (dialer) {
			dialer->Dispose();
		}
		if (peer) {
			peer->Dispose();

		}

		serviceDialerPeers.erase(iter);
		return 0;
	}


	///***********************************************************************************************/
	//// web ���
	///***********************************************************************************************/

	//// �ȴ� ����� �Ľ���
	//std::shared_ptr<xx::HttpListener> webListener;

	//// ��ַ path : ������ ӳ����䵽��
	//std::unordered_map<std::string, std::function<int(xx::HttpContext & request, xx::HttpResponse & response)>> handlers;

	//void InitWebListener();
};

//inline void Gateway::InitWebListener() {
//	// ���� web listener( ֻ֧�� tcp )
//	xx::MakeTo(webListener, , cfg.webListenIP, cfg.webListenPort);
//
//	webListener->onAccept = [this](xx::HttpPeer_s peer) {
//		peer->onReceiveHttp = [this, peer](xx::HttpContext& request, xx::HttpResponse& response)->int {
//			// ��� request.path ��
//			request.ParseUrl();
//
//			// �� path ���Ҵ�����
//			auto&& iter = handlers.find(request.path);
//
//			// ���û�ҵ������Ĭ�ϱ���ҳ��
//			if (iter == handlers.end()) {
//				response.Send404Body("the page not found!");
//			}
//			// �ҵ���ִ��
//			else {
//				// ���ִ�г������Ĭ�ϱ���ҳ��
//				if (iter->second(request, response)) {
//					response.Send404Body("bad request!");
//				}
//			}
//			return 0;
//		};
//	};
//
//	handlers[""] = [](xx::HttpContext& request, xx::HttpResponse& r)->int {
//		char const str[] = R"--(
//<html><body>
//<p><a href="/watch_service_cfg">�鿴 cfg</a></p>
//<p><a href="/watch_connect_info">�鿴 ����״̬</a></p>
//<p><a href="/reload_service_cfg">���¼��������ļ�</a></p>
//</body></html>
//)--";
//		return r.onSend(r.prefixHtml, str, sizeof(str));
//	};
//
//	handlers["watch_service_cfg"] = [this](xx::HttpContext& request, xx::HttpResponse& r)->int {
//		{
//			auto&& html_body = r.Scope("<html><body>", "</body></html>");
//
//			r.Tag("p", "gatewayId = ", cfg.gatewayId);
//			r.Tag("p", "gatewayId = ", cfg.gatewayId);
//			r.Tag("p", "listenIP = ", cfg.listenIP);
//			r.Tag("p", "listenPort = ", cfg.listenPort);
//			r.Tag("p", "listenTcpKcpOpt = ", cfg.listenTcpKcpOpt);
//			r.Tag("p", "clientTimeoutMS = ", cfg.clientTimeoutMS);
//			r.Tag("p", "webListenIP = ", cfg.webListenIP);
//			r.Tag("p", "webListenPort = ", cfg.webListenPort);
//
//			r.TableBegin("serviceId", "ip", "port");
//			for (auto&& service : cfg.services) {
//				r.TableRow(service.serviceId, service.ip, service.port);
//			}
//			r.TableEnd();
//
//			r.A("�ص����˵�", "/");
//		}
//		return r.Send();
//	};
//
//	handlers["watch_connect_info"] = [this](xx::HttpContext& request, xx::HttpResponse& r)->int {
//		{
//			auto&& html = r.Scope("html");
//			auto&& body = r.Scope("body");
//
//			r.A("ˢ��", "/watch_connected_services");
//
//			r.P("clientPeers.size() = ", clientPeers.size());
//
//			r.TableBegin("serviceId", "ip:port", "busy", "peer alive");
//			for (auto&& kv : serviceDialerPeers) {
//				auto&& dialer = kv.second.first;
//				auto&& peer = kv.second.second;
//				r.TableRow(
//					dialer->serviceId
//					, (dialer->ip + ":" + std::to_string(dialer->port))
//					, (dialer->Busy() ? "<font color='red'>true</font>" : "false")
//					, ((peer && !peer->Disposed()) ? "<font color='green'>true</font>" : "false"));
//			}
//			r.TableEnd();
//
//			r.A("�ص����˵�", "/");
//		}
//		return r.Send();
//	};
//
//	handlers["reload_service_cfg"] = [this](xx::HttpContext& request, xx::HttpResponse& response)->int {
//		ServiceCfg cfg2;
//		ajson::load_from_file(cfg2, "service_cfg.json");
//
//		// gatewayId, listenIP�� listenPort��listenTcpKcpOpt, webListenIP, webListenPort ���ɱ�, Ҳ�����
//		cfg.clientTimeoutMS = cfg2.clientTimeoutMS;
//
//		// ��ʼ�Ա�. foreach old
//		for (auto&& service : cfg.services) {
//			// �� serviceId ��Ϊƥ������
//			auto&& iter = std::find_if(cfg2.services.begin(), cfg2.services.end(), [&](ServiceInfo const& o) {
//				return service.serviceId == o.serviceId;
//				});
//
//			// ���û�ҵ���ɾ����� dialer & peer
//			if (iter == cfg2.services.end()) {
//				RemoveServiceDialerPeer(service.serviceId);
//			}
//			// ����ҵ����� ip ��˿��б仯, �� remove + add
//			else if (service.ip != iter->ip || service.port != iter->port) {
//				RemoveServiceDialerPeer(iter->serviceId);
//				TryCreateServiceDialer(iter->serviceId, iter->ip, iter->port);
//			}
//		}
//
//		// �ٴζԱ�. foreach new
//		for (auto&& service : cfg2.services) {
//			// �� serviceId ��Ϊƥ������
//			auto&& iter = std::find_if(cfg.services.begin(), cfg.services.end(), [&](ServiceInfo const& o) {
//				return service.serviceId == o.serviceId;
//				});
//
//			// ���û�ҵ���add
//			if (iter == cfg.services.end()) {
//				TryCreateServiceDialer(service.serviceId, service.ip, service.port);
//			}
//		}
//
//		// ���� cfg
//		cfg.services = cfg2.services;
//
//		// ��ʾ������
//		return handlers["watch_service_cfg"](request, response);
//	};
//}


template<typename BaseListener>
void FromClientListener<BaseListener>::OnAccept(EP::Ref<typename BaseListener::PeerType> const& peer) {
	// ��������ʱ�������� id �����ֵ� ��������Ӧ�¼��������
	auto&& cp = (FromClientPeer<typename BaseListener::PeerType>*)peer.Lock();
	assert(cp);

	// ���Ĭ��ת���������δ����������������
	auto&& sp_0 = gw->serviceDialerPeers[0].second;
	if (!sp_0) {
#if PRINT_LOG_SERVICE0_NOT_READY
		xx::CoutN("service 0 is not ready");
#endif
		return;
	}

	// �������� id, ����ӳ���ֵ�
	gw->clientPeerAutoId += 2;
	if constexpr (std::is_base_of_v<EP::KcpListener, BaseListener>) {
		cp->clientId = gw->clientPeerAutoId + 1;
		gw->clientKcpPeers.emplace(cp->clientId, cp);
	}
	else {
		cp->clientId = gw->clientPeerAutoId;
		gw->clientTcpPeers.emplace(cp->clientId, cp);
	}

	// ע���¼�������ʱ���ֵ��Ƴ�
	cp->onDisconnect = [gw = this->gw, cp]{
		assert(cp->clientId);
		if constexpr (std::is_base_of_v<EP::KcpListener, BaseListener>) {
			gw->clientKcpPeers.erase(cp->clientId);
		}
		else {
			gw->clientTcpPeers.erase(cp->clientId);
		}

		// Ⱥ���Ͽ�֪ͨ
		cp->serviceIds.emplace(0);	// ȷ���� 0 service Ҳ�㲥
		for (auto&& serviceId : cp->serviceIds) {
			if (auto&& sp = gw->serviceDialerPeers[serviceId].second.Lock()) {
				sp->SendCommand_Disconnect(cp->clientId);
			}
		}
#if PRINT_LOG_CLIENT_PEER_DISCONNECT
		xx::CoutN("client peer disconnect: ", cp->addr);
#endif
	};

	// ע���¼����յ��ͻ��˷�����ָ�ֱ�� echo ����
	cp->onReceiveCommand = [gw = this->gw, cp](uint8_t* const& buf, std::size_t const& len) {
		// ����
		cp->SetTimeout(gw->cfg.clientTimeoutMS / 10);
		// echo ����
		cp->Send((char*)buf - 4, len + 4);
	};

	// ע���¼����յ�����֮����� serviceId ���ֲ���λ�� service peer ת��
	cp->onReceivePackage = [gw = this->gw, cp](uint8_t* const& buf, std::size_t const& len) {
		// ȡ�� serviceId
		if (len < 4) {
			cp->Dispose();
			return;
		}
		auto serviceId = *(uint32_t*)buf;

#if PRINT_LOG_CP_SENDTO_SP
		xx::CoutN("cp -> sp. size = ", len, ", serviceId = ", serviceId);
#endif

		// �жϸ÷������Ƿ��ڰ�������. �Ҳ�����Ͽ�
		if (cp->serviceIds.find(serviceId) == cp->serviceIds.end()) {
			cp->Dispose();
			return;
		}

		// ���Ҷ�Ӧ�� servicePeer
		auto&& sp = gw->serviceDialerPeers[serviceId].second.Lock();

		// ���δӳ����ѶϿ� �ͷ��ش����룬�⽫���� client peer �Ͽ�
		if (!sp) {
			cp->Dispose();
			return;
		}

		// ����. ÿ���յ��Ϸ�������һ��
		cp->SetTimeout(gw->cfg.clientTimeoutMS / 10);

		// �۸� serviceId Ϊ clientId, ת������ header ������
		*(uint32_t*)buf = cp->clientId;
		sp->Send((char*)buf - 4, len + 4);
	};

	// ����. ���Ӻ� xx MS �����û���յ��κ����ݣ����ӽ��Ͽ�
	cp->SetTimeout(gw->cfg.clientTimeoutMS / 10);

	// ��Ĭ�Ϸ����� accept ֪ͨ
	std::string ip;
	xx::Append(ip, cp->addr);
	sp_0->SendCommand_Accept(cp->clientId, ip);

#if PRINT_LOG_CLIENT_PEER_ACCEPT
	xx::CoutN("client peer accept: ", cp->addr, ", protocol = ", (std::is_base_of_v<EP::KcpListener, BaseListener> ? "kcp" : "tcp"));
#endif
	}


int main() {
	xx::IgnoreSignal();
	Gateway g;
	g.ep.Run(100);
	return 0;
}
