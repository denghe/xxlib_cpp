//#include <xx__http.h>
#include <xx_epoll2.hpp>
#include <xx_epoll2_http.h>
#include <unordered_set>
namespace EP = xx::Epoll;

#define PRINT_LOG_SERVICE_CONFIG			0
#define PRINT_LOG_SERVICE0_NOT_READY		1
#define PRINT_LOG_SERVICE_CONNECTD			1
#define PRINT_LOG_SERVICE_DISCONNECTD		1
#define PRINT_LOG_SERVICE_DIAL				0

#define PRINT_LOG_CLIENT_PEER_ACCEPT		0
#define PRINT_LOG_CLIENT_PEER_DISCONNECT	0

#define PRINT_LOG_CP_SENDTO_SP				0
#define PRINT_LOG_SP_SENDTO_CP				0

#define PRINT_LOG_RECV_OPEN					0
#define PRINT_LOG_RECV_CLOSE				0
#define PRINT_LOG_RECV_KICK					0



/***********************************************************************************************************************/
// 配置文件相关
/***********************************************************************************************************************/

#include"ajson.hpp"

struct ServiceInfo {
	int serviceId = 0;
	std::string ip;
	int port = 0;
};
AJSON(ServiceInfo, serviceId, ip, port);

struct ServiceCfg {
	int gatewayId = 0;						// 当前网关内部编号
	std::string listenIP;					// 监听到哪个 ip 上( 通常为 0.0.0.0 )
	int listenPort = 0;						// 监听端口
	int listenTcpKcpOpt = 0;				// 协议选择: 0: tcp only;  1: kcp ony;   2: tcp + kcp
	int clientTimeoutMS = 0;				// 客户端掉线检测时长 ms. 超出这个时间没有收到客户端的合法数据则会断掉客户端
	std::string webListenIP;				// 监视器监听到哪个 ip 上( 通常为 0.0.0.0 )
	int webListenPort = 0;					// 监听端口
	std::vector<ServiceInfo> services;		// 要连接到哪些服务
};
AJSON(ServiceCfg, gatewayId, listenIP, listenPort, listenTcpKcpOpt, clientTimeoutMS, webListenIP, webListenPort, services);




/***********************************************************************************************************************/
// 通信 peer 相关
/***********************************************************************************************************************/

// 可继承自 TcpPeer 或 KcpPeer. 扩展高阶功能
template<typename Base>
struct Peer : Base {
	// 处理内部指令包( 4 字节长度 + 0xFFFFFFFF + 内容( 通常为 cmd string + args... ) )
	std::function<void(uint8_t* const& buf, std::size_t const& len)> onReceiveCommand;

	// 处理一般数据包( 4 字节长度 + 4字节地址 + 数据 )
	std::function<void(uint8_t* const& buf, std::size_t const& len)> onReceivePackage;

	// 处理断线事件
	std::function<void()> onDisconnect;

	// 开始向 bb 写包. 空出 长度 头部
	static void WritePackageBegin(xx::BBuffer& bb, size_t const& reserveLen, uint32_t const& addr) {
		bb.Reserve(4 + reserveLen);
		bb.len = 4;
		bb.WriteFixed(addr);
	}

	// 结束 bb 写包。根据数据长度 填写 包头
	static void WritePackageEnd(xx::BBuffer& bb) {
		*(uint32_t*)bb.buf = (uint32_t)(bb.len - 4);
	}

	// 构造内部指令包. cmd string + args...
	template<typename...Args>
	void SendCommand(Args const& ... cmdAndArgs) {
		auto&& bb = this->ep->sendBB;
		WritePackageBegin(bb, 1024, 0xFFFFFFFFu);
		bb.Write(cmdAndArgs...);
		WritePackageEnd(bb);
		this->Send(bb);
	}

	virtual void OnReceive() override {
		// 取出指针备用
		auto buf = (uint8_t*)this->recv.buf;
		auto end = (uint8_t*)this->recv.buf + this->recv.len;

		// 如果是 kcp 则会收到一次触发 accept 行为的包
		if constexpr (std::is_base_of_v<EP::KcpPeer, Base>) {
			// 如果内容符合 1 0 0 0 0 打头则跳过这部分
			if (this->recv.len >= 5 && *(uint32_t*)buf == 1 && buf[4] == 0) {
				buf += 5;
			}
		}

		// 确保包头长度充足
		while (buf + 4 <= end) {

			// 取出数据区长度 并 判断合法性
			auto dataLen = *(uint32_t*)buf;
			if (dataLen > this->ep->maxPackageLength) {
				this->Dispose();
				return;
			}

			// 数据区不完整就退出
			if (buf + 4 + dataLen > end) break;

			// 跳到数据区开始调用处理回调
			buf += 4;
			{
				// 死亡判断变量
				EP::Ref<Base> alive(this);

				// 判断是否为内部指令
				if (*(uint32_t*)buf == 0xFFFFFFFFu) {
					if (onReceiveCommand) {
						onReceiveCommand(buf + 4, dataLen - 4);
					}
				}
				else {
					if (onReceivePackage) {
						onReceivePackage(buf, dataLen);
					}
				}

				// 如果当前类实例已自杀则退出
				if (!alive) return;
			}
			// 跳到下一个包的开头
			buf += dataLen;
		}

		// 移除掉已处理的数据
		this->recv.RemoveFront((char*)buf - this->recv.buf);
	}

	virtual void OnDisconnect(int const& reason) override {
		if (onDisconnect) {
			onDisconnect();
		}
	}
};

// 可继承自 TcpPeer 或 KcpPeer
template<typename Base>
struct FromClientPeer : Peer<Base> {
	// 自增编号, accept 时填充
	uint32_t clientId = 0xFFFFFFFFu;

	// 允许访问的 service peers 的 id 的白名单
	std::unordered_set<uint32_t> serviceIds;

	void SendCommand_Open(uint32_t const& serviceId) {
		this->SendCommand("open", serviceId);
	}

	void SendCommand_Close(uint32_t const& serviceId) {
		this->SendCommand("close", serviceId);
	}
};

struct ToServicePeer : Peer<EP::TcpPeer> {
	// 内部服务编号, 从配置填充
	uint32_t serviceId = 0xFFFFFFFFu;

	// 等待 ping 回包的状态值
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
	// 记录当连接成功时 要映射到的 服务 id
	uint32_t serviceId = 0;

	virtual EP::Peer_u OnCreatePeer(EP::Protocol const& protocol) override {
		assert(protocol == EP::Protocol::Tcp);
		return xx::TryMakeU<ToServicePeer>();
	}

	std::function<void(EP::Ref<ToServicePeer> const& peer)> onConnect;
	virtual void OnConnect(EP::Peer_r const& peer) override {
		onConnect(peer.As<ToServicePeer>());
	}
};

// 可继承自 TcpListener 或 KcpListener
template<typename BaseListener>
struct FromClientListener : BaseListener {
	// 指向服务总上下文
	Gateway* gw = nullptr;

	virtual std::unique_ptr<typename BaseListener::PeerType> OnCreatePeer() override {
		return xx::TryMakeU<FromClientPeer<typename BaseListener::PeerType>>();
	}

	virtual void OnAccept(EP::Ref<typename BaseListener::PeerType> const& peer) override;
};




/***********************************************************************************************************************/
// 网关主体
/***********************************************************************************************************************/

struct Gateway {
	EP::Context ep;

	// 服务启动配置. 从 json 加载
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
		InitWebListener();
		InitCmds();
	}


	/***********************************************************************************************/
	// client
	/***********************************************************************************************/

	// 等待 client 的接入
	EP::Ref<FromClientListener<EP::TcpListener>> clientTcpListener;
	EP::Ref<FromClientListener<EP::KcpListener>> clientKcpListener;

	// clientPeerAutoId += 2;  new cp.clientId = clientPeerAutoId + kcp ? 1 : 0
	uint32_t clientPeerAutoId = 0;

	// key: clientId. kcp client id 为单数
	std::unordered_map<uint32_t, EP::Ref<FromClientPeer<EP::TcpPeer>>> clientTcpPeers;
	std::unordered_map<uint32_t, EP::Ref<FromClientPeer<EP::KcpPeer>>> clientKcpPeers;

	void InitClientListener() {
		// 创建 listener
		clientTcpListener = ep.CreateTcpListener<FromClientListener<EP::TcpListener>>(cfg.listenPort);
		clientKcpListener = ep.CreateUdpPeer<FromClientListener<EP::KcpListener>>(cfg.listenPort);

		if (!clientTcpListener) throw - 1;
		if (!clientKcpListener) throw - 2;

		clientTcpListener->gw = this;
		clientKcpListener->gw = this;
	}



	/***********************************************************************************************/
	// service
	/***********************************************************************************************/

	// 拨号器和 peer 存储在一起，绑定关系
	using DialerPeer = std::pair<EP::Ref<ToServiceDialer>, EP::Ref<ToServicePeer>>;

	// key: serviceId
	std::unordered_map<uint32_t, DialerPeer> serviceDialerPeers;


	void InitServiceDialers() {
		// 创建拨号用 timer
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

		// 创建 dialers
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

		// 设置相应事件处理代码
		dialer->onConnect = [this, serviceId](auto peer) {
			// 如果没连上, 忽略
			auto&& sp = peer.Lock();
			if (!sp) return;

			// 设置其 serviceId
			sp->serviceId = serviceId;

			// 放入容器
			serviceDialerPeers[serviceId].second = sp;

			// 注册事件：断开时清除相应 peer 存储变量
			sp->onDisconnect = [this, sp] {

				// 从存储区移除
				this->serviceDialerPeers[sp->serviceId].second.Reset();

				// 从所有 client peers 里的白名单中移除 并自动下发 close.	// todo: 如果  白名单 空了，直接物理断开
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

			// 注册事件：收到推送的处理
			sp->onReceivePackage = [this, sp](uint8_t* const& buf, std::size_t const& len) {
				// 读出 clientId
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
					// 如果没找到 或已断开 则返回，忽略错误
					auto&& iter = this->clientKcpPeers.find(clientId);
					if (iter == this->clientKcpPeers.end()) return;
					auto&& cp = iter->second.Lock();
					if (!cp) return;

					// 篡改 clientId 为 serviceId 转发( 带 header )
					*(uint32_t*)buf = sp->serviceId;
					cp->Send((char*)buf - 4, len + 4);
				}
				else {
					// 如果没找到 或已断开 则返回，忽略错误
					auto&& iter = this->clientTcpPeers.find(clientId);
					if (iter == this->clientTcpPeers.end()) return;
					auto&& cp = iter->second.Lock();
					if (!cp) return;

					// 篡改 clientId 为 serviceId 转发( 带 header )
					*(uint32_t*)buf = sp->serviceId;
					cp->Send((char*)buf - 4, len + 4);
				}
			};

			// 注册事件：收到内部指令的处理
			sp->onReceiveCommand = [this, sp](uint8_t* const& buf, std::size_t const& len) {
				// 搞个 bb 容器方便点
				auto&& bb = ep.recvBB;
				bb.Reset(buf, len);

				// 试读取 cmd 字串
				std::string cmd;
				if (int r = bb.Read(cmd)) {
					sp->Dispose();
					return;
				}

				// 开端口. 参数: clientId
				if (cmd == "open") {
					// 试读出 clientId
					uint32_t clientId = 0;
					if (int r = bb.Read(clientId)) {
						sp->Dispose();
						return;
					}

					// kcp
					if (clientId & 1) {
						// 如果没找到 或已断开 则返回，忽略错误
						auto&& iter = this->clientKcpPeers.find(clientId);
						if (iter == this->clientKcpPeers.end()) return;
						auto&& cp = iter->second.Lock();
						if (!cp) return;

						// 放入白名单
						cp->serviceIds.emplace(sp->serviceId);

						// 下发 open
						cp->SendCommand_Open(sp->serviceId);
					}
					else {
						// 如果没找到 或已断开 则返回，忽略错误
						auto&& iter = this->clientTcpPeers.find(clientId);
						if (iter == this->clientTcpPeers.end()) return;
						auto&& cp = iter->second.Lock();
						if (!cp) return;

						// 放入白名单
						cp->serviceIds.emplace(sp->serviceId);

						// 下发 open
						cp->SendCommand_Open(sp->serviceId);
					}

#if PRINT_LOG_RECV_OPEN
					xx::CoutN("service peer recv cmd open: clientId: ", clientId, ", serviceId = ", sp->serviceId);
#endif
				}


				// 关端口. 参数: clientId
				else if (cmd == "close") {
					// 试读出 clientId
					uint32_t clientId = 0;
					if (int r = bb.Read(clientId)) {
						sp->Dispose();
						return;
					}

					// 前置检查
					if (!clientId) {
						sp->Dispose();
						return;
					}

					// kcp
					if (clientId & 1) {
						// 如果没找到 或已断开 则返回，忽略错误
						auto&& iter = this->clientKcpPeers.find(clientId);
						if (iter == this->clientKcpPeers.end()) return;
						auto&& cp = iter->second.Lock();
						if (!cp) return;

						// 从白名单移除
						cp->serviceIds.erase(sp->serviceId);

						// 下发 close
						cp->SendCommand_Close(sp->serviceId);
					}
					else {
						// 如果没找到 或已断开 则返回，忽略错误
						auto&& iter = this->clientTcpPeers.find(clientId);
						if (iter == this->clientTcpPeers.end()) return;
						auto&& cp = iter->second.Lock();
						if (!cp) return;

						// 从白名单移除
						cp->serviceIds.erase(sp->serviceId);

						// 下发 close
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
				// 踢玩家下线. 参数: clientId, delayMS
				else if (cmd == "kick") {
					// 试读出参数
					uint32_t clientId = 0;
					int64_t delayMS = 0;
					if (int r = bb.Read(clientId, delayMS)) {
						sp->Dispose();
						return;
					}

					// 前置检查
					if (!clientId) {
						sp->Dispose();
						return;
					}

					// kcp
					if (clientId & 1) {
						// 如果没找到 或已断开 则返回，忽略错误
						auto&& iter = this->clientKcpPeers.find(clientId);
						if (iter == this->clientKcpPeers.end()) return;
						auto&& cp = iter->second.Lock();
						if (!cp) return;

						if (delayMS) {
							// 追加一个 close 指令以便 client 收到后直接自杀
							cp->SendCommand_Close(0);

							// 延迟断开，先解绑事件处理函数，再设置超时时长，到时会 Dispose()
							auto cp1 = cp;							// onDisconnect 会导致 cp 变量失效故复制
							cp1->onDisconnect();					// 解除映射并发送断线通知
							cp1->onDisconnect = [cp1] {};			// 清除旧函数并持有
							cp1->onReceivePackage = nullptr;
							cp1->SetTimeout(ep.MsToFrames((int)delayMS));
						}
						else {
							// 立刻断开连接，触发 onDisconnect( 从 this->clientPeers 移除并向白名单 serviceIds 对应 peer 广播断开通知 )
							cp->Dispose();
						}
					}
					else {
						// 如果没找到 或已断开 则返回，忽略错误
						auto&& iter = this->clientTcpPeers.find(clientId);
						if (iter == this->clientTcpPeers.end()) return;
						auto&& cp = iter->second.Lock();
						if (!cp) return;

						if (delayMS) {
							// 追加一个 close 指令以便 client 收到后直接自杀
							cp->SendCommand_Close(0);

							// 延迟断开，先解绑事件处理函数，再设置超时时长，到时会 Dispose()
							auto cp1 = cp;							// onDisconnect 会导致 cp 变量失效故复制
							cp1->onDisconnect();					// 解除映射并发送断线通知
							cp1->onDisconnect = [cp1] {};			// 清除旧函数并持有
							cp1->onReceivePackage = nullptr;
							cp1->SetTimeout(ep.MsToFrames((int)delayMS));
						}
						else {
							// 立刻断开连接，触发 onDisconnect( 从 this->clientPeers 移除并向白名单 serviceIds 对应 peer 广播断开通知 )
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

			// 向 service 发送自己的 gatewayId
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


	/***********************************************************************************************/
	// web 相关
	/***********************************************************************************************/

	// 等待 浏览器 的接入
	EP::Ref<EP::HttpListener> webListener;

	void InitWebListener();



	/***********************************************************************************************/
	// 命令行 相关
	/***********************************************************************************************/

	void InitCmds();

	void ReloadConfig();
};

inline void Gateway::InitWebListener() {
	// 创建 web listener( 只支持 tcp )
	webListener = ep.CreateTcpListener<EP::HttpListener>(cfg.webListenPort);
	if (!webListener) throw - 1;

	webListener->handlers[""] = [](xx::HttpContext& request, xx::HttpResponse& r) {
		char const str[] = R"--(
<html><body>
<p><a href="/show_service_cfg">查看 cfg</a></p>
<p><a href="/show_connect_info">查看 连接状态</a></p>
<p><a href="/reload_service_cfg">重新加载配置文件</a></p>
</body></html>
)--";
		r.onSend(r.prefixHtml, str, sizeof(str));
	};

	webListener->handlers["show_service_cfg"] = [this](xx::HttpContext& request, xx::HttpResponse& r) {
		{
			auto&& html_body = r.Scope("<html><body>", "</body></html>");

			r.Tag("p", "gatewayId = ", cfg.gatewayId);
			r.Tag("p", "listenIP = ", cfg.listenIP);
			r.Tag("p", "listenPort = ", cfg.listenPort);
			r.Tag("p", "listenTcpKcpOpt = ", cfg.listenTcpKcpOpt);
			r.Tag("p", "clientTimeoutMS = ", cfg.clientTimeoutMS);
			r.Tag("p", "webListenIP = ", cfg.webListenIP);
			r.Tag("p", "webListenPort = ", cfg.webListenPort);

			r.TableBegin("serviceId", "ip", "port");
			for (auto&& service : cfg.services) {
				r.TableRow(service.serviceId, service.ip, service.port);
			}
			r.TableEnd();

			r.A("回到主菜单", "/");
		}
		r.Send();
	};

	webListener->handlers["show_connect_info"] = [this](xx::HttpContext& request, xx::HttpResponse& r) {
		{
			auto&& html = r.Scope("html");
			auto&& body = r.Scope("body");

			r.A("刷新", "/show_connect_info");

			r.P("clientTcpPeers.size() = ", clientTcpPeers.size());
			r.P("clientKcpPeers.size() = ", clientKcpPeers.size());

			r.TableBegin("serviceId", "ip:port", "busy", "peer alive");
			for (auto&& kv : serviceDialerPeers) {
				auto&& dialer = kv.second.first;
				auto&& peer = kv.second.second;
				r.TableRow(
					dialer->serviceId
					, dialer->addrs[0].first
					, (dialer->Busy() ? "<font color='red'>true</font>" : "false")
					, (peer ? "<font color='green'>true</font>" : "false"));
			}
			r.TableEnd();

			r.A("回到主菜单", "/");
		}
		r.Send();
	};

	webListener->handlers["reload_service_cfg"] = [this](xx::HttpContext& request, xx::HttpResponse& response) {
		this->ReloadConfig();

		// 显示新配置
		this->webListener->handlers["show_service_cfg"](request, response);
	};
}

inline void Gateway::ReloadConfig() {
	ServiceCfg cfg2;
	ajson::load_from_file(cfg2, "service_cfg.json");

	// gatewayId, listenIP， listenPort，listenTcpKcpOpt, webListenIP, webListenPort 不可变, 也不检查
	cfg.clientTimeoutMS = cfg2.clientTimeoutMS;

	// 开始对比. foreach old
	for (auto&& service : cfg.services) {
		// 按 serviceId 作为匹配条件
		auto&& iter = std::find_if(cfg2.services.begin(), cfg2.services.end(), [&](ServiceInfo const& o) {
			return service.serviceId == o.serviceId;
			});

		// 如果没找到：删掉相关 dialer & peer
		if (iter == cfg2.services.end()) {
			RemoveServiceDialerPeer(service.serviceId);
		}
		// 如果找到但是 ip 或端口有变化, 则 remove + add
		else if (service.ip != iter->ip || service.port != iter->port) {
			RemoveServiceDialerPeer(iter->serviceId);
			TryCreateServiceDialer(iter->serviceId, iter->ip, iter->port);
		}
	}

	// 再次对比. foreach new
	for (auto&& service : cfg2.services) {
		// 按 serviceId 作为匹配条件
		auto&& iter = std::find_if(cfg.services.begin(), cfg.services.end(), [&](ServiceInfo const& o) {
			return service.serviceId == o.serviceId;
			});

		// 如果没找到：add
		if (iter == cfg.services.end()) {
			TryCreateServiceDialer(service.serviceId, service.ip, service.port);
		}
	}

	// 更新 cfg
	cfg.services = cfg2.services;
}


inline void Gateway::InitCmds() {
	// 启用配置文件变化监视
	auto&& inh = ep.CreateINotifyHandler("./service_cfg.json", IN_CLOSE_WRITE);
	if (inh) {
		inh->onEvent = [this](auto fn, auto mask) {
			this->ReloadConfig();
			xx::CoutN("event: service_cfg.json changed. reload success.");
		};
	}
	else {
		xx::CoutN("warning: service_cfg.json changed monitor create failed.");
	}

	// 启用命令行支持
	ep.EnableCommandLine();

	ep.cmds["exit"] = [this](auto args) {
		ep.running = false;
	};
	ep.cmds["reload"] = [this](auto args) {
		this->ReloadConfig();
		xx::CoutN("reload success.");
	};
	ep.cmds["config"] = [this](auto args) {
		xx::CoutN("gatewayId = ", cfg.gatewayId
			, "\nlistenIP = ", cfg.listenIP
			, "\nlistenPort = ", cfg.listenPort
			, "\nlistenTcpKcpOpt = ", cfg.listenTcpKcpOpt
			, "\nclientTimeoutMS = ", cfg.clientTimeoutMS
			, "\nwebListenIP = ", cfg.webListenIP
			, "\nwebListenPort = ", cfg.webListenPort);

		xx::CoutN("services:");
		for (auto&& service : cfg.services) {
			xx::CoutN("   id = ", service.serviceId, ", ip = ", service.ip, ", port = ", service.port);
		}
	};
	ep.cmds["info"] = [this](auto args) {
		xx::CoutN("clientTcpPeers.size() = ", clientTcpPeers.size());
		xx::CoutN("clientKcpPeers.size() = ", clientKcpPeers.size());

		xx::CoutN("serviceId		ip:port		busy		peer alive");
		for (auto&& kv : serviceDialerPeers) {
			auto&& dialer = kv.second.first;
			auto&& peer = kv.second.second;
			xx::CoutN(
				dialer->serviceId
				, "\t\t", dialer->addrs[0].first
				, "\t\t", (dialer->Busy() ? "true" : "false")
				, "\t\t", (peer ? "true" : "false"));
		}
	};
}



template<typename BaseListener>
void FromClientListener<BaseListener>::OnAccept(EP::Ref<typename BaseListener::PeerType> const& peer) {
	// 接受连接时分配自增 id 放入字典 并设置相应事件处理代码
	auto&& cp = (FromClientPeer<typename BaseListener::PeerType>*)peer.Lock();
	assert(cp);

	// 如果默认转发处理服务未就绪，不接受连接
	auto&& sp_0 = gw->serviceDialerPeers[0].second;
	if (!sp_0) {
#if PRINT_LOG_SERVICE0_NOT_READY
		xx::CoutN("service 0 is not ready");
#endif
		return;
	}

	// 产生自增 id, 放入映射字典
	gw->clientPeerAutoId += 2;
	if constexpr (std::is_base_of_v<EP::KcpListener, BaseListener>) {
		cp->clientId = gw->clientPeerAutoId + 1;
		gw->clientKcpPeers.emplace(cp->clientId, cp);
	}
	else {
		cp->clientId = gw->clientPeerAutoId;
		gw->clientTcpPeers.emplace(cp->clientId, cp);
	}

	// 注册事件：断线时从字典移除
	cp->onDisconnect = [gw = this->gw, cp]{
		assert(cp->clientId);
		if constexpr (std::is_base_of_v<EP::KcpListener, BaseListener>) {
			gw->clientKcpPeers.erase(cp->clientId);
		}
		else {
			gw->clientTcpPeers.erase(cp->clientId);
		}

		// 群发断开通知
		cp->serviceIds.emplace(0);	// 确保向 0 service 也广播
		for (auto&& serviceId : cp->serviceIds) {
			if (auto&& sp = gw->serviceDialerPeers[serviceId].second.Lock()) {
				sp->SendCommand_Disconnect(cp->clientId);
			}
		}
#if PRINT_LOG_CLIENT_PEER_DISCONNECT
		xx::CoutN("client peer disconnect: ", cp->addr);
#endif
	};

	// 注册事件：收到客户端发来的指令，直接 echo 返回
	cp->onReceiveCommand = [gw = this->gw, cp](uint8_t* const& buf, std::size_t const& len) {
		// 续命
		cp->SetTimeout(gw->ep.MsToFrames(gw->cfg.clientTimeoutMS));
		// echo 发回( buf 指向了 header + 0xffffffff 之后的区域，故 -8 指向 header )
		cp->Send((char*)buf - 8, len + 8);
	};

	// 注册事件：收到数据之后解析 serviceId 部分并定位到 service peer 转发
	cp->onReceivePackage = [gw = this->gw, cp](uint8_t* const& buf, std::size_t const& len) {
		// 取出 serviceId
		if (len < 4) {
			cp->Dispose();
			return;
		}
		auto serviceId = *(uint32_t*)buf;

#if PRINT_LOG_CP_SENDTO_SP
		xx::CoutN("cp -> sp. size = ", len, ", serviceId = ", serviceId);
#endif

		// 判断该服务编号是否在白名单中. 找不到则断开
		if (cp->serviceIds.find(serviceId) == cp->serviceIds.end()) {
			cp->Dispose();
			return;
		}

		// 查找对应的 servicePeer
		auto&& sp = gw->serviceDialerPeers[serviceId].second.Lock();

		// 如果未映射或已断开 就返回错误码，这将导致 client peer 断开
		if (!sp) {
			cp->Dispose();
			return;
		}

		// 续命. 每次收到合法数据续一下
		cp->SetTimeout(gw->ep.MsToFrames(gw->cfg.clientTimeoutMS));

		// 篡改 serviceId 为 clientId, 转发包含 header 的整包
		*(uint32_t*)buf = cp->clientId;
		sp->Send((char*)buf - 4, len + 4);
	};

	// 续命. 连接后 xx MS 内如果没有收到任何数据，连接将断开
	cp->SetTimeout(gw->ep.MsToFrames(gw->cfg.clientTimeoutMS));

	// 向默认服务发送 accept 通知
	std::string ip;
	xx::Append(ip, cp->addr);
	sp_0->SendCommand_Accept(cp->clientId, ip);

#if PRINT_LOG_CLIENT_PEER_ACCEPT
	xx::CoutN("client peer accept: ", cp->addr, ", protocol = ", (std::is_base_of_v<EP::KcpListener, BaseListener> ? "kcp" : "tcp"));
#endif
}


int main() {
	xx::IgnoreSignal();
	xx::CoutN("running... press TAB 2 times show commands.");
	Gateway g;
	g.ep.Run(100);
	return 0;
}
