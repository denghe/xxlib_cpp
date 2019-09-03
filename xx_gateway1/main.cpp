#include <xx_uv_ext.h>
#include <unordered_set>

struct UvGatewayPeer : xx::UvCommandPeer {
	using UvCommandPeer::UvCommandPeer;

	// 直接投递原始 buf 方便小改转发
	std::function<int(uint8_t* const& buf, size_t const& len)> onReceive;

	inline virtual bool Dispose(int const& flag = 1) noexcept override {
		if (!this->UvCommandPeer::Dispose(flag)) return false;
		if (flag == -1) return true;
		auto holder = shared_from_this();
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

	int SendCommand_GatewayId(uint32_t const& gatewayId) {
		return SendCommand("gatewayId", gatewayId);
	}

	int SendCommand_Accept(uint32_t const& clientId, std::string const& ip) {
		return SendCommand("accept", clientId, ip);
	}

	int SendCommand_Disconnect(uint32_t const& clientId) {
		return SendCommand("disconnect", clientId);
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

struct Gateway {
	static_assert(sizeof(UvToServicePeer::serviceId) == sizeof(UvFromClientPeer::clientId));
	xx::Uv uv;

	// 代理内部编号( 可能来自配置文件或启动参数等 )
	uint32_t gatewayId = 1;

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
		xx::MakeTo(clientListener, uv, "0.0.0.0", 20000, 2);

		// 接受连接时分配自增 id 放入字典 并设置相应事件处理代码
		clientListener->onAccept = [this](xx::UvPeer_s peer) {
			// 转为正确的类型
			auto&& cp = xx::As<UvFromClientPeer>(peer);

			// 如果默认转发处理服务未就绪，不接受连接
			auto&& sp_0 = serviceDialerPeers[0].second;
			if (!sp_0 || sp_0->Disposed()) {
				xx::CoutN("service 0 is not ready");
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

				xx::CoutN("client peer disconnect: ", cp->GetIP());
			};

			// 注册事件：收到数据之后解析 serviceId 部分并定位到 service peer 转发
			cp->onReceive = [this, cp](uint8_t* const& buf, size_t const& len)->int {
				uint32_t serviceId = 0;

				// 取出 serviceId
				if (len < sizeof(serviceId)) return -1;
				::memcpy(&serviceId, buf, sizeof(serviceId));

				// 判断该服务编号是否在白名单中. 找不到则断开
				if (cp->serviceIds.find(serviceId) == cp->serviceIds.end()) return -1;

				// 查找对应的 servicePeer
				auto&& sp = serviceDialerPeers[serviceId].second;

				//// 如果未映射或已断开 就返回错误码，这将导致 client peer 断开
				if (!sp || sp->Disposed()) return -2;

				// 续命 5 秒. 每次收到合法数据续一下
				cp->ResetTimeoutMS(5000);

				// 篡改 serviceId 为 clientId, 转发包含 header 的整包
				::memcpy(buf, &cp->clientId, sizeof(serviceId));
				return sp->SendDirect(buf - 4, len + 4);
			};

			// 续命 5 秒. 连接后 5 秒内如果没有收到任何数据，连接将断开
			cp->ResetTimeoutMS(5000);

			// 向默认服务发送 accept 通知
			sp_0->SendCommand_Accept(cp->clientId, cp->GetIP());

			xx::CoutN("client peer accept: ", cp->GetIP(), ", protocol = ", (cp->IsKcp() ? "kcp" : "tcp"));
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
						xx::CoutN("service dialer dial...");
					}
				}
			}
			});


		// todo: 根据配置来得到 要连接的 service 的明细. 配置可能是经由某个内部服务来获取
		// 创建几个 dialer	
		TryCreateServiceDialer(0, "127.0.0.1", 22222);
	}

	int TryCreateServiceDialer(uint32_t const& serviceId, std::string const& ip, int const& port) {
		auto&& dialer = serviceDialerPeers[serviceId].first;
		if (dialer) return -1;

		xx::TryMakeTo(dialer, uv);
		if (!dialer) return -2;

		dialer->serviceId = 0;
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

				// 从所有 client peers 里的白名单中移除 并自动下发 close. 如果  白名单 空了，直接物理断开
				for (auto&& kv : clientPeers) {
					if (kv.second && !kv.second->Disposed()) {
						kv.second->SendCommand_Close(sp->serviceId);
					}
				}

				xx::CoutN("service peer disconnect: ", sp->GetIP(), ", serviceId = ", sp->serviceId);
			};

			// 注册事件：收到推送的处理
			sp->onReceive = [this, sp](uint8_t* const& buf, size_t const& len)->int {
				// 读出 clientId
				uint32_t clientId = 0;
				if (len < sizeof(clientId)) return -1;
				::memcpy(&clientId, buf, sizeof(clientId));

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

					// 前置检查
					if (!clientId) return -1;

					// 如果没找到 或已断开 则返回，忽略错误
					auto&& iter = this->clientPeers.find(clientId);
					if (iter == this->clientPeers.end()) return 0;
					auto&& cp = iter->second;
					if (!cp || cp->Disposed()) return 0;

					// 放入白名单
					cp->serviceIds.emplace(sp->serviceId);

					// 下发 open
					cp->SendCommand_Open(sp->serviceId);

					xx::CoutN("gateway service peer recv cmd open: clientId: ", clientId, ", serviceId = ", sp->serviceId);
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

					xx::CoutN("gateway service peer recv cmd close: clientId: ", clientId, ", serviceId = ", sp->serviceId);
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
						// 延迟断开，先解绑事件处理函数，再设置超时时长，到时会 Dispose()
						cp->onDisconnect();						// 解除映射并发送断线通知
						cp->onDisconnect = [cp] {};				// 清除旧函数并持有
						cp->onReceive = nullptr;
						cp->ResetTimeoutMS(delayMS);
					}
					else {
						// 立刻断开连接，触发 onDisconnect( 从 this->clientPeers 移除并向白名单 serviceIds 对应 peer 广播断开通知 )
						cp->Dispose();
					}

					xx::CoutN("gateway service peer recv cmd kick: clientId: ", clientId);
					return 0;
				}

				else {
					return -1;
				}
			};

			// 向 service 发送自己的 gatewayId
			sp->SendCommand_GatewayId(gatewayId);

			xx::CoutN("service peer connect: ", sp->GetIP());
		};

		return dialer->Dial();
	}


	/***********************************************************************************************/
	// constructor
	/***********************************************************************************************/

	Gateway() {
		InitClientListener();
		InitServiceDialers();
	}
};

int main() {
	Gateway g;
	g.uv.Run();
	return 0;
}
