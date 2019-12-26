#include <xx_uv_ext.h>

// 模拟了一个 game server. 连到 service0. 被 gateway 连

template<typename CMD, typename ...Args>
xx::BBuffer_s& WriteCmd(xx::BBuffer_s& bb, CMD const& cmd, Args const&...args) {
	if (!bb) {
		xx::MakeTo(bb);
	}
	else {
		bb->Clear();
	}
	bb->Write(cmd, args...);
	return bb;
}

template<typename ...Args>
xx::BBuffer_s& WriteCmd_Error(xx::BBuffer_s& bb, Args const&...args) {
	std::string s;
	xx::Append(s, args...);
	return WriteCmd(bb, "error", s);
}

template<typename Peer, typename Cmd, typename ...Args>
int SendResponse(Peer& peer, int const& serial, xx::BBuffer_s& bb, Cmd const& cmd, Args const&...args) {
	if (!peer || peer->Disposed()) return -1;
	return peer->SendResponse(serial, WriteCmd(bb, cmd, args...));
}

template<typename Peer, typename ...Args>
int SendResponse_Error(Peer& peer, int const& serial, xx::BBuffer_s& bb, Args const&...args) {
	if (!peer || peer->Disposed()) return -1;
	return peer->SendResponse(serial, WriteCmd_Error(bb, args...));
}


using PeerType = xx::UvSimulatePeer;
struct GameServer : xx::UvServiceBase<PeerType, true> {

	xx::UvPeer_s service0peer;
	xx::UvDialer_s dialer;
	xx::UvTimer_s timer;

	GameServer() {
		// 初始化用于 gateway 连接的 listener
		InitGatewayListener("0.0.0.0", 10002);

		// 初始化用于 连接到 service0 的 dialer
		xx::MakeTo(dialer, uv, 0);
		dialer->onConnect = [this](xx::UvPeer_s peer) {
			// 没连上
			if (!peer) return;

			// 存连接到上下文备用
			this->service0peer = peer;

			// 注册 peer 请求处理
			peer->onReceiveRequest = [this, peer](int const& serial, xx::Object_s&& msg) {
				// 当前就是直接用 bb 来收发数据. 如果不是这个就报错
				auto&& bb = xx::As<xx::BBuffer>(msg);
				if (!bb) {
					return SendResponse_Error(peer, serial, bb, "msg is not bb");
				}

				// 当前就是以 cmd string + args... 作为数据格式. 先读 cmd. 再根据具体 cmd 读具体参数
				std::string cmd;
				// 读错误则返回错误
				if (auto r = bb->Read(cmd)) {
					return SendResponse_Error(peer, serial, bb, "msg read cmd error. r = ", r);
				}

				// 处理进入指令. 继续读出 gatewayId, clientId 并通知相应的 gateway open
				if (cmd == "enter") {
					uint32_t gatewayId = 0, clientId = 0;
					// 读错误则返回错误
					if (auto r = bb->Read(gatewayId, clientId)) {
						return SendResponse_Error(peer, serial, bb, "cmd: enter read serviceId, clientId error. r = ", r);
					}

					// 开始联系网关 open
					auto iter = gatewayPeers.find(gatewayId);

					// 找不到 或 未连接 或 已断开: 返回错误信息
					if (iter == gatewayPeers.end() || !iter->second || iter->second->Disposed()) {
						return SendResponse_Error(peer, serial, bb, "cmd: enter can't find gateway. gatewayId = ", gatewayId);
					}
					// 向网关发送 open cmd
					else {
						(void)iter->second->SendCommand_Open(clientId);
					}

					// 创建虚拟 peer ( 如果已存在就会被顶下线 )
					auto&& cp = iter->second->CreateSimulatePeer<PeerType>(clientId);
					AcceptSimulatePeer(cp);

					// 返回成功
					return SendResponse(peer, serial, bb, "success");
				}
				else {
					// 返回错误: 收到未处理的指令
					return SendResponse_Error(peer, serial, bb, "recv unhandled cmd: ", cmd);
				}

				return 0;
			};

			// 发送 register + serviceId
			auto&& bb = xx::Make<xx::BBuffer>();
			bb->Write("register", (uint32_t)1);
			(void)peer->SendRequest(bb, [](xx::Object_s&& msg) {
				// 当前就是直接用 bb 来收发数据. 如果不是这个就报错
				auto&& bb = xx::As<xx::BBuffer>(msg);
				if (!bb) {
					xx::CoutN("register recv null.");
					return -1;
				}
				
				// 当前就是以 cmd string + args 作为数据格式. 先读 cmd
				std::string cmd;
				if (auto r = bb->Read(cmd)) {
					xx::CoutN("register read cmd error. r = ", r);
					return -2;
				}

				// 成功
				if (cmd == "success") {
					xx::CoutN("register success.");
					return 0;
				}
				// 收到错误提示
				else if (cmd == "error") {
					std::string errText;
					if (auto r = bb->Read(errText)) {
						xx::CoutN("register read errText error. r = ", r);
						return -3;
					}
					xx::CoutN("register recv error: ", errText);
					return -4;
				}
				// 收到意料之外的包
				else {
					xx::CoutN("recv unhandled command: ", cmd);
					return -5;
				}
			}, 5000);
		};

		xx::MakeTo(timer, uv, 0, 500, [this] {
			if (!dialer->Busy() && (!service0peer || service0peer->Disposed())) {
				dialer->Dial("192.168.1.132", 10011);
			}
		});
	}

	virtual void AcceptSimulatePeer(std::shared_ptr<PeerType>& sp) override {
		xx::CoutN("AcceptSimulatePeer");
		// 实现 echo 效果
		sp->onReceiveRequest = [sp](int const& serial, xx::Object_s&& msg)->int {
			xx::CoutN("recv request msg = ", msg);
			return sp->SendResponse(serial, msg);
		};
		sp->onReceivePush = [sp](xx::Object_s&& msg)->int {
			xx::CoutN("recv push msg = ", msg);
			return sp->SendPush(msg);
		};
	}
};

int main() {
	GameServer s;
	s.uv.Run();
	return 0;
}
