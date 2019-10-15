#include <xx_uv_ext.h>
#include <unordered_set>

struct UvFrontPear : xx::UvCommandPeer {
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

struct UvFromClientPeer : UvFrontPear {
	using UvFrontPear::UvFrontPear;

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

struct UvToRearPeer : UvFrontPear {
	using UvFrontPear::UvFrontPear;

	int SendCommand_Accept(uint32_t const& clientId, std::string const& ip) {
		return SendCommand("accept", clientId, ip);
	}

	int SendCommand_Disconnect(uint32_t const& clientId) {
		return SendCommand("disconnect", clientId);
	}
};

struct UvToRearDialer : xx::UvDialer {
	UvToRearDialer(xx::Uv& uv)
		: UvDialer(uv, 0) {
		this->onCreatePeer = [](xx::Uv& uv) {
			return xx::TryMake<UvToRearPeer>(uv);
		};
	}

	// 需要保存下来的信息，反复 Dial 或 onConnect 时可以方便的读取
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

struct Front {
	xx::Uv uv;

	/***********************************************************************************************/
	// client
	/***********************************************************************************************/

	// 等待 client 的接入
	std::shared_ptr<UvFromClientListener> clientListener;

	// new cp.clientId = ++clientPeerAutoId
	uint32_t clientPeerAutoId = 0;

	// key: clientId
	std::unordered_map<uint32_t, std::shared_ptr<UvFromClientPeer>> clientPeers;

	// 临时用于那啥
	std::vector<uint8_t> tmpBufPlaceHolder;
	uint8_t* tmpBuf = nullptr;
	std::size_t tmpBufLen = 0;

	void InitVars() {
		tmpBufPlaceHolder.resize(uv.maxPackageLength + 8);	// 8: len + clientId
		tmpBuf = tmpBufPlaceHolder.data();
		tmpBufLen = tmpBufPlaceHolder.size();
	}

	void InitClientListener() {
		// 创建 listener( tcp, kcp 同时支持 )
		xx::MakeTo(clientListener, uv, "0.0.0.0", 20000, 2);

		// 接受连接时分配自增 id 放入字典 并设置相应事件处理代码
		clientListener->onAccept = [this](xx::UvPeer_s peer) {
			if (!rearPeerAlive()) {
				xx::CoutN("rear is not ready");
				return;
			}

			// 转为正确的类型
			auto&& cp = xx::As<UvFromClientPeer>(peer);

			// 产生自增 id
			cp->clientId = ++clientPeerAutoId;

			// 放入映射字典
			clientPeers.emplace(cp->clientId, cp);

			// 注册事件：断线时从字典移除
			cp->onDisconnect = [this, cp] {
				assert(cp->clientId);
				this->clientPeers.erase(cp->clientId);

				// 发断开通知( rear 那边也有 serviceIds. 收到该指令后 继续群发断开通知 )
				if (rearPeerAlive()) {
					rearPeer->SendCommand_Disconnect(cp->clientId);
				}

				xx::CoutN("client peer disconnect: ", cp->GetIP());
			};

			// 注册事件：收到数据之后转发给 rear
			cp->onReceive = [this, cp](uint8_t* const& buf, std::size_t const& len)->int {
				assert(len + 8 <= tmpBufLen);

				// 如果到 rear 的连接不正常则断开
				if (!rearPeerAlive()) return -2;

				// 取出 serviceId
				uint32_t serviceId = 0;
				if (len < sizeof(serviceId)) return -1;
				::memcpy(&serviceId, buf, sizeof(serviceId));

				// 判断该服务编号是否在白名单中. 找不到则断开
				if (cp->serviceIds.find(serviceId) == cp->serviceIds.end()) return -1;

				// 填充包长度( buf 负区间有 8 字节预留 )
				*(uint32_t*)(buf - 8) = (uint32_t)(len + 4);

				// 填充 senderId
				*(uint32_t*)(buf - 4) = cp->clientId;

				// 续命 5 秒. 每次收到合法数据续一下
				cp->ResetTimeoutMS(5000);

				// 发出. 总长度为 sizeof(header) + sizeof(senderId) + len
				return rearPeer->SendDirect(buf - 8, len + 8);
			};

			// 续命 5 秒. 连接后 5 秒内如果没有收到任何数据，连接将断开
			cp->ResetTimeoutMS(5000);

			// 向 rear 发送 accept 通知( rear 继续向默认服务转发 )
			rearPeer->SendCommand_Accept(cp->clientId, cp->GetIP());

			xx::CoutN("client peer accept: ", cp->GetIP(), ", protocol = ", (cp->IsKcp() ? "kcp" : "tcp"));
		};
	}



	/***********************************************************************************************/
	// real 相关
	/***********************************************************************************************/

	std::shared_ptr<xx::UvTimer> rearDialerTimer;
	std::shared_ptr<UvToRearDialer> rearDialer;
	std::shared_ptr<UvToRearPeer> rearPeer;
	bool rearPeerAlive() {
		return rearPeer && !rearPeer->Disposed();
	}

	void InitServiceDialers() {
		// 创建拨号用 timer
		xx::MakeTo(rearDialerTimer, uv, 500, 500, [this] {
			if (!rearPeerAlive()) {
				if (!rearDialer->Busy()) {
					rearDialer->Dial();
					xx::CoutN("rear rearDialer dial...");
				}
			}
		});

		// todo: 根据配置来得到 要连接的 rear 的明细. 配置可能是经由某个内部服务来获取
		TryCreateRearDialer("127.0.0.1", 22220);
	}

	int TryCreateRearDialer(std::string const& ip, int const& port) {
		if (rearPeer) return -1;

		xx::TryMakeTo(rearDialer, uv);
		if (!rearDialer) return -2;

		rearDialer->ip = ip;
		rearDialer->port = port;

		// 设置相应事件处理代码
		rearDialer->onConnect = [this](xx::UvPeer_s peer) {
			// 如果没连上, 忽略
			if (!peer) return;

			// 还原出原始类型备用
			rearPeer = xx::As<UvToRearPeer>(peer);

			// 注册事件：rear 断开时清除存储以及断开所有 client
			rearPeer->onDisconnect = [this] {
				this->rearPeer.reset();
				clientPeers.clear();
				xx::CoutN("rear peer disconnect.");
			};

			// 注册事件：收到推送的处理
			rearPeer->onReceive = [this](uint8_t* const& buf, std::size_t const& len)->int {
				// 读出 serviceId
				uint32_t serviceId = 0;
				if (len < sizeof(serviceId)) return -1;
				::memcpy(&serviceId, buf, sizeof(serviceId));

				// 读出 clientId
				uint32_t clientId = 0;
				if (len < sizeof(clientId)) return -1;
				::memcpy(&clientId, buf + 4, sizeof(clientId));

				// 如果 相应 client 没找到 或已断开 则返回，忽略错误
				auto&& iter = this->clientPeers.find(clientId);
				if (iter == this->clientPeers.end()) return 0;
				auto&& cp = iter->second;
				if (!cp || cp->Disposed()) return 0;

				// 篡改 serviceId 为 包长
				auto dataLen = (uint32_t)(len - 4);
				::memcpy(buf, &dataLen, sizeof(dataLen));

				// 篡改 clientId 为 serviceId
				::memcpy(buf + 4, &serviceId, sizeof(serviceId));

				// 发出
				(void)cp->SendDirect(buf, len);
				return 0;
			};

			// 注册事件：收到内部指令的处理
			rearPeer->onReceiveCommand = [this](xx::BBuffer& bb)->int {
				// 试读取 cmd 字串
				std::string cmd;
				if (int r = bb.Read(cmd)) return r;

				// 开端口. 参数: clientId, serviceId
				if (cmd == "open") {
					// 试读出 clientId, serviceId
					uint32_t clientId = 0, serviceId = 0;
					if (int r = bb.Read(clientId, serviceId)) return r;

					// 如果没找到 或已断开 则返回，忽略错误
					auto&& iter = this->clientPeers.find(clientId);
					if (iter == this->clientPeers.end()) return 0;
					auto&& cp = iter->second;
					if (!cp || cp->Disposed()) return 0;

					// 放入白名单
					cp->serviceIds.emplace(serviceId);

					// 下发 open
					cp->SendCommand_Open(serviceId);

					xx::CoutN("gateway service peer recv cmd open: clientId: ", clientId, ", serviceId = ", serviceId);
					return 0;
				}


				// 关端口. 参数: clientId, serviceId
				else if (cmd == "close") {
					// 试读出 clientId, serviceId
					uint32_t clientId = 0, serviceId = 0;
					if (int r = bb.Read(clientId, serviceId)) return r;

					// 如果没找到 或已断开 则返回，忽略错误
					auto&& iter = this->clientPeers.find(clientId);
					if (iter == this->clientPeers.end()) return 0;
					auto&& cp = iter->second;
					if (!cp || cp->Disposed()) return 0;

					// 从白名单移除
					cp->serviceIds.erase(serviceId);

					// 下发 close
					cp->SendCommand_Close(serviceId);

					xx::CoutN("gateway service peer recv cmd close: clientId: ", clientId, ", serviceId = ", serviceId);
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

			xx::CoutN("rear peer connect: ", rearPeer->GetIP());
		};

		return rearDialer->Dial();
	}


	/***********************************************************************************************/
	// constructor
	/***********************************************************************************************/

	Front() {
		InitVars();
		InitClientListener();
		InitServiceDialers();
	}
};

int main() {
	Front g;
	g.uv.Run();
	return 0;
}
