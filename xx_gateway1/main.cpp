#include <xx_uv_ext.h>
#include <xx_uv_http.h>
#include <unordered_set>


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

struct WebHandler;



/***********************************************************************************************************************/
// 通信 peer 相关
/***********************************************************************************************************************/

struct UvGatewayPeer : xx::UvCommandPeer {
	using UvCommandPeer::UvCommandPeer;

	// 直接投递原始 buf 方便小改转发
	std::function<int(uint8_t* const& buf, std::size_t const& len)> onReceive;

	inline virtual bool Dispose(int const& flag = 1) noexcept override {
		if (Disposed()) return false;
		xx::UvItem_s holder;
		if (flag != -1) {
			holder = shared_from_this();
		}
		this->UvCommandPeer::Dispose(flag);
		if (flag == -1) return true;
		onReceive = nullptr;
		onReceiveCommand = nullptr;
		return true;
	}

protected:
	inline virtual int HandlePack(uint8_t* const& recvBuf, uint32_t const& recvLen) noexcept override {
		// for kcp listener accept
		if (recvLen == 1 && *recvBuf == 0) {
			ip = peerBase->GetIP();
			return 0;
		}

		auto& bb = uv.recvBB;
		bb.Reset(recvBuf, recvLen);

		uint32_t id = 0;
		if (int r = bb.ReadFixed(id)) return r;
		if (id == 0xFFFFFFFFu) {
			return onReceiveCommand ? onReceiveCommand(bb) : 0;
		}

		return onReceive ? onReceive(recvBuf, recvLen) : 0;
	}
};

struct UvFromClientPeer : UvGatewayPeer {
	using UvGatewayPeer::UvGatewayPeer;

	// 自增编号, accept 时填充
	uint32_t clientId = 0xFFFFFFFFu;

	// 允许访问的 service peers 的 id 的白名单
	std::unordered_set<uint32_t> serviceIds;

	int SendCommand_Open(uint32_t const& serviceId) {
		return SendCommand("open", serviceId);
	}

	int SendCommand_Close(uint32_t const& serviceId) {
		return SendCommand("close", serviceId);
	}
};

struct UvToServicePeer : UvGatewayPeer {
	using UvGatewayPeer::UvGatewayPeer;

	// 内部服务编号, 从配置填充
	uint32_t serviceId = 0xFFFFFFFFu;
	bool waitPingBack = false;
	int SendCommand_GatewayId(uint32_t const& gatewayId) {
		return SendCommand("gatewayId", gatewayId);
	}

	int SendCommand_Accept(uint32_t const& clientId, std::string const& ip) {
		return SendCommand("accept", clientId, ip);
	}

	int SendCommand_Disconnect(uint32_t const& clientId) {
		return SendCommand("disconnect", clientId);
	}
	int SendCommand_Ping() {
		return SendCommand("ping", xx::NowEpoch10m());
	}
};

struct UvToServiceDialer : xx::UvDialer {
	UvToServiceDialer(xx::Uv& uv)
		: UvDialer(uv, 0) {
		this->onCreatePeer = [](xx::Uv& uv) {
			return xx::TryMake<UvToServicePeer>(uv);
		};
	}

	// 需要保存下来的信息，反复 Dial 或 onConnect 时可以方便的读取
	uint32_t serviceId = 0;
	std::string ip;
	int port = 0;

	int Dial(uint64_t const& timeoutMS = 2000) {
		return this->UvDialer::Dial(ip, port, timeoutMS);
	}
};

struct UvFromClientListener : xx::UvListener {
	using UvListener::UvListener;
	virtual xx::UvPeer_s CreatePeer() noexcept override {
		return xx::TryMake<UvFromClientPeer>(uv);
	}
};




/***********************************************************************************************************************/
// 网关主体
/***********************************************************************************************************************/

struct Gateway {
	static_assert(sizeof(UvToServicePeer::serviceId) == sizeof(UvFromClientPeer::clientId));
	xx::Uv uv;

	// 服务启动配置. 从 json 加载
	ServiceCfg cfg;

	xx::UvTimer_s pingTimer;
	/***********************************************************************************************/
	// constructor
	/***********************************************************************************************/
	Gateway() {
		ajson::load_from_file(cfg, "service_cfg.json");

		xx::MakeTo(pingTimer, uv, 1000, 5 * 1000, [this]() {
			for (auto&& kv : serviceDialerPeers) {
				auto&& peer = kv.second.second;
				if (peer && !peer->Disposed()) {
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
			});
		InitClientListener();
		InitServiceDialers();
		InitWebListener();
	}


	/***********************************************************************************************/
	// client
	/***********************************************************************************************/

	// 等待 client 的接入
	std::shared_ptr<UvFromClientListener> clientListener;

	// new cp.clientId = ++clientPeerAutoId
	uint32_t clientPeerAutoId = 0;

	// key: clientId
	std::unordered_map<uint32_t, std::shared_ptr<UvFromClientPeer>> clientPeers;

	void InitClientListener() {
		// 创建 listener( tcp, kcp 同时支持 )
		xx::MakeTo(clientListener, uv, cfg.listenIP, cfg.listenPort, cfg.listenTcpKcpOpt);

		// 接受连接时分配自增 id 放入字典 并设置相应事件处理代码
		clientListener->onAccept = [this](xx::UvPeer_s peer) {
			// 转为正确的类型
			auto&& cp = xx::As<UvFromClientPeer>(peer);

			// 如果默认转发处理服务未就绪，不接受连接
			auto&& sp_0 = serviceDialerPeers[0].second;
			if (!sp_0 || sp_0->Disposed()) {
#if PRINT_LOG_SERVICE0_NOT_READY
				xx::CoutN("service 0 is not ready");
#endif
				return;
			}

			// 产生自增 id
			cp->clientId = ++clientPeerAutoId;

			// 放入映射字典
			clientPeers.emplace(cp->clientId, cp);

			// 注册事件：断线时从字典移除
			cp->onDisconnect = [this, cp] {
				assert(cp->clientId);
				this->clientPeers.erase(cp->clientId);

				// 群发断开通知
				cp->serviceIds.emplace(0);	// 确保向 0 service 也广播
				for (auto&& serviceId : cp->serviceIds) {
					auto&& sp = serviceDialerPeers[serviceId].second;
					if (sp && !sp->Disposed()) {
						sp->SendCommand_Disconnect(cp->clientId);
					}
				}
#if PRINT_LOG_CLIENT_PEER_DISCONNECT
				xx::CoutN("client peer disconnect: ", cp->GetIP());
#endif
			};

			// 注册事件：收到客户端发来的指令，直接 echo 返回
			cp->onReceiveCommand = [this, cp](xx::BBuffer& bb)->int {
				// 续命
				cp->ResetTimeoutMS(cfg.clientTimeoutMS);
				// echo 发回
				return cp->SendDirect(bb.buf - 4, bb.len + 4);
			};

			// 注册事件：收到数据之后解析 serviceId 部分并定位到 service peer 转发
			cp->onReceive = [this, cp](uint8_t* const& buf, std::size_t const& len)->int {
				uint32_t serviceId = 0;

				// 取出 serviceId
				if (len < sizeof(serviceId)) return -1;
				::memcpy(&serviceId, buf, sizeof(serviceId));

#if PRINT_LOG_CP_SENDTO_SP
				xx::CoutN("cp -> sp. size = ", len, ", serviceId = ", serviceId);
#endif

				// 判断该服务编号是否在白名单中. 找不到则断开
				if (cp->serviceIds.find(serviceId) == cp->serviceIds.end()) return -1;

				// 查找对应的 servicePeer
				auto&& sp = serviceDialerPeers[serviceId].second;

				// 如果未映射或已断开 就返回错误码，这将导致 client peer 断开
				if (!sp || sp->Disposed()) return -2;

				// 续命. 每次收到合法数据续一下
				cp->ResetTimeoutMS(cfg.clientTimeoutMS);

				// 篡改 serviceId 为 clientId, 转发包含 header 的整包
				::memcpy(buf, &cp->clientId, sizeof(serviceId));
				return sp->SendDirect(buf - 4, len + 4);
			};

			// 续命. 连接后 xx MS 内如果没有收到任何数据，连接将断开
			cp->ResetTimeoutMS(cfg.clientTimeoutMS);

			// 向默认服务发送 accept 通知
			sp_0->SendCommand_Accept(cp->clientId, cp->GetIP());

#if PRINT_LOG_CLIENT_PEER_ACCEPT
			xx::CoutN("client peer accept: ", cp->GetIP(), ", protocol = ", (cp->IsKcp() ? "kcp" : "tcp"));
#endif
		};

	}



	/***********************************************************************************************/
	// service
	/***********************************************************************************************/

	// 拨号用 timer
	std::shared_ptr<xx::UvTimer> serviceDialTimer;

	// 拨号器和 peer 存储在一起，绑定关系
	using DialerPeer = std::pair<std::shared_ptr<UvToServiceDialer>, std::shared_ptr<UvToServicePeer>>;

	// key: serviceId
	std::unordered_map<uint32_t, DialerPeer> serviceDialerPeers;


	void InitServiceDialers() {
		// 创建拨号用 timer
		xx::MakeTo(serviceDialTimer, uv, 500, 500, [this] {
			for (auto&& kv : serviceDialerPeers) {
				//auto&& serviceId = kv.first;
				auto&& dialer = kv.second.first;
				auto&& peer = kv.second.second;
				if (!peer || peer->Disposed()) {
					if (!dialer->Busy()) {
						dialer->Dial();
#if PRINT_LOG_SERVICE_DIAL
						xx::CoutN("service dialer dial...");
#endif
					}
				}
			}
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

		xx::TryMakeTo(dialer, uv);
		if (!dialer) return -2;

		dialer->serviceId = serviceId;
		dialer->ip = ip;
		dialer->port = port;

		// 设置相应事件处理代码
		dialer->onConnect = [this, serviceId](xx::UvPeer_s peer) {
			// 如果没连上, 忽略
			if (!peer) return;

			// 还原出原始类型备用
			auto&& sp = xx::As<UvToServicePeer>(peer);

			// 设置其 serviceId
			sp->serviceId = serviceId;

			// 放入容器
			serviceDialerPeers[serviceId].second = sp;

			// 注册事件：断开时清除相应 peer 存储变量
			sp->onDisconnect = [this, sp] {

				// 从存储区移除
				this->serviceDialerPeers[sp->serviceId].second.reset();

				// 从所有 client peers 里的白名单中移除 并自动下发 close.	// todo: 如果  白名单 空了，直接物理断开
				for (auto&& kv : clientPeers) {
					if (kv.second && !kv.second->Disposed()) {
						kv.second->SendCommand_Close(sp->serviceId);
					}
				}

#if PRINT_LOG_SERVICE_DISCONNECTD
				xx::CoutN("service peer disconnect: ", sp->GetIP(), ", serviceId = ", sp->serviceId);
#endif
			};

			// 注册事件：收到推送的处理
			sp->onReceive = [this, sp](uint8_t* const& buf, std::size_t const& len)->int {
				// 读出 clientId
				uint32_t clientId = 0;
				if (len < sizeof(clientId)) return -1;
				::memcpy(&clientId, buf, sizeof(clientId));

#if PRINT_LOG_SP_SENDTO_CP
				xx::CoutN("sp -> cp. size = ", len, ", clientId = ", clientId);
#endif

				// 如果没找到 或已断开 则返回，忽略错误
				auto&& iter = this->clientPeers.find(clientId);
				if (iter == this->clientPeers.end()) return 0;
				auto&& cp = iter->second;
				if (!cp || cp->Disposed()) return 0;

				// 篡改 clientId 为 serviceId 转发( 带 header )
				::memcpy(buf, &sp->serviceId, sizeof(sp->serviceId));
				(void)cp->SendDirect(buf - 4, len + 4);
				return 0;
			};

			// 注册事件：收到内部指令的处理
			sp->onReceiveCommand = [this, sp](xx::BBuffer& bb)->int {
				// 试读取 cmd 字串
				std::string cmd;
				if (int r = bb.Read(cmd)) return r;

				// 开端口. 参数: clientId
				if (cmd == "open") {
					// 试读出 clientId
					uint32_t clientId = 0;
					if (int r = bb.Read(clientId)) return r;

					// 如果没找到 或已断开 则返回，忽略错误
					auto&& iter = this->clientPeers.find(clientId);
					if (iter == this->clientPeers.end()) return 0;
					auto&& cp = iter->second;
					if (!cp || cp->Disposed()) return 0;

					// 放入白名单
					cp->serviceIds.emplace(sp->serviceId);

					// 下发 open
					cp->SendCommand_Open(sp->serviceId);

#if PRINT_LOG_RECV_OPEN
					xx::CoutN("service peer recv cmd open: clientId: ", clientId, ", serviceId = ", sp->serviceId);
#endif
					return 0;
				}


				// 关端口. 参数: clientId
				else if (cmd == "close") {
					// 试读出 clientId
					uint32_t clientId = 0;
					if (int r = bb.Read(clientId)) return r;

					// 前置检查
					if (!clientId) return -1;

					// 如果没找到 或已断开 则返回，忽略错误
					auto&& iter = this->clientPeers.find(clientId);
					if (iter == this->clientPeers.end()) return 0;
					auto&& cp = iter->second;
					if (!cp || cp->Disposed()) return 0;

					// 从白名单移除
					cp->serviceIds.erase(sp->serviceId);

					// 下发 close
					cp->SendCommand_Close(sp->serviceId);

#if PRINT_LOG_RECV_CLOSE
					xx::CoutN("service peer recv cmd close: clientId: ", clientId, ", serviceId = ", sp->serviceId);
#endif
					return 0;
				}
				else if (cmd == "ping") {

					sp->waitPingBack = false;
					return 0;
				}
				// 踢玩家下线. 参数: clientId, delayMS
				else if (cmd == "kick") {
					// 试读出参数
					uint32_t clientId = 0;
					int64_t delayMS = 0;
					if (int r = bb.Read(clientId, delayMS)) return r;

					// 前置检查
					if (!clientId) return -1;

					// 如果没找到 或已断开 则返回，忽略错误
					auto&& iter = this->clientPeers.find(clientId);
					if (iter == this->clientPeers.end()) return 0;
					auto&& cp = iter->second;
					if (!cp || cp->Disposed()) return 0;

					if (delayMS) {
						// 追加一个 close 指令以便 client 收到后直接自杀
						if (auto r = cp->SendCommand_Close(0)) return r;

						// 延迟断开，先解绑事件处理函数，再设置超时时长，到时会 Dispose()
						auto cp1 = cp;							// onDisconnect 会导致 cp 变量失效故复制
						cp1->onDisconnect();					// 解除映射并发送断线通知
						cp1->onDisconnect = [cp1] {};			// 清除旧函数并持有
						cp1->onReceive = nullptr;
						cp1->ResetTimeoutMS(delayMS);
					}
					else {
						// 立刻断开连接，触发 onDisconnect( 从 this->clientPeers 移除并向白名单 serviceIds 对应 peer 广播断开通知 )
						cp->Dispose();
					}

#if PRINT_LOG_RECV_KICK
					xx::CoutN("service peer recv cmd kick: clientId: ", clientId);
#endif
					return 0;
				}

				else {
					return -1;
				}
			};

			// 向 service 发送自己的 gatewayId
			sp->SendCommand_GatewayId(cfg.gatewayId);

#if PRINT_LOG_SERVICE_CONNECTD
			xx::CoutN("service peer connected to: ", sp->GetIP(), ", serviceId = ", sp->serviceId);
#endif
		};

		return dialer->Dial();
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
	std::shared_ptr<xx::UvHttpListener> webListener;

	// 网址 path : 处理函数 映射填充到此
	std::unordered_map<std::string, std::function<int(xx::HttpContext & request, xx::HttpResponse & response)>> handlers;

	void InitWebListener();
};

inline void Gateway::InitWebListener() {
	// 创建 web listener( 只支持 tcp )
	xx::MakeTo(webListener, uv, cfg.webListenIP, cfg.webListenPort);

	webListener->onAccept = [this](xx::UvHttpPeer_s peer) {
		peer->onReceiveHttp = [this, peer](xx::HttpContext& request, xx::HttpResponse& response)->int {
			// 填充 request.path 等
			request.ParseUrl();

			// 用 path 查找处理函数
			auto&& iter = handlers.find(request.path);

			// 如果没找到：输出默认报错页面
			if (iter == handlers.end()) {
				response.Send404Body("the page not found!");
			}
			// 找到则执行
			else {
				// 如果执行出错，输出默认报错页面
				if (iter->second(request, response)) {
					response.Send404Body("bad request!");
				}
			}
			return 0;
		};
	};

	handlers[""] = [](xx::HttpContext& request, xx::HttpResponse& r)->int {
		char const str[] = R"--(
<html><body>
<p><a href="/watch_service_cfg">查看 cfg</a></p>
<p><a href="/watch_connected_services">查看 内部服务连接状态</a></p>
<p><a href="/reload_service_cfg">重新加载配置文件</a></p>
</body></html>
)--";
		return r.onSend(r.prefixHtml, str, sizeof(str));
	};

	handlers["watch_service_cfg"] = [this](xx::HttpContext& request, xx::HttpResponse& r)->int {
		{
			auto&& html_body = r.Scope("<html><body>", "</body></html>");

			r.Tag("p", "gatewayId = ", cfg.gatewayId);
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
		return r.Send();
	};

	handlers["watch_connected_services"] = [this](xx::HttpContext& request, xx::HttpResponse& r)->int {
		{
			auto&& html = r.Scope("html");
			auto&& body = r.Scope("body");

			r.TableBegin("serviceId", "ip:port", "busy", "peer alive");
			for (auto&& kv : serviceDialerPeers) {
				auto&& dialer = kv.second.first;
				auto&& peer = kv.second.second;
				r.TableRow(
					dialer->serviceId
					, (dialer->ip + ":" + std::to_string(dialer->port))
					, dialer->Busy()
					, (peer && !peer->Disposed()));
			}
			r.TableEnd();

			r.A("回到主菜单", "/");
		}
		return r.Send();
	};

	handlers["reload_service_cfg"] = [this](xx::HttpContext& request, xx::HttpResponse& response)->int {
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

		// 显示新配置
		return handlers["watch_service_cfg"](request, response);
	};
}




int main() {
	Gateway g;
	g.uv.Run();
	return 0;
}
