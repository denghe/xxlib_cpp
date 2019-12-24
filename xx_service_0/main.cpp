#include <xx_uv_ext.h>

// 模拟了一个 0 号服务. 被 gateway & game server 连

template<typename Cmd, typename ...Args>
xx::BBuffer_s& WriteCmd(xx::BBuffer_s& bb, Cmd const& cmd, Args const&...args) {
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
struct Service0 : xx::UvServiceBase<PeerType, true> {

	// 被 game server 连
	xx::UvListener_s serviceListener;
	std::unordered_map<uint32_t, xx::UvPeer_s> servicePeers;

	Service0() {
		// 初始化用于 gateway 连接的 listener
		InitGatewayListener("0.0.0.0", 10001);

		// 初始化用于 game service 连接的 listener
		xx::MakeTo(serviceListener, uv, "0.0.0.0", 10011, 0);
		serviceListener->onAccept = [this](xx::UvPeer_s peer) {
			xx::CoutN("server accept: ", peer->GetIP());
			peer->onReceiveRequest = [this, peer](int const& serial, xx::Object_s&& msg)->int {
				// 当前就是直接用 bb 来收发数据. 如果不是这个就报错
				auto&& bb = xx::As<xx::BBuffer>(msg);
				if (!bb) {
					return SendResponse_Error(peer, serial, bb, "msg is not bb");
				}

				// 当前就是以 cmd string + args... 作为数据格式. 先读 cmd. 再根据具体 cmd 读具体参数
				std::string cmd;
				// 读错误则返回错误
				if (auto r = bb->Read(cmd)) {
					return SendResponse_Error(peer, serial, bb, "msg read cmd error, r = ", r);
				}

				// 如果是注册包. 继续读 serviceId
				if (cmd == "register") {
					uint32_t serviceId = 0;
					// 读错误则返回错误
					if (auto r = bb->Read(serviceId)) {
						return SendResponse_Error(peer, serial, bb, "cmd: register read serviceId error. r = ", r);
					}

					// 如果已注册 且 连接健康 则返回错误
					auto&& iter = servicePeers.find(serviceId);
					if (iter != servicePeers.end() && iter->second && !iter->second->Disposed()) {
						return SendResponse_Error(peer, serial, bb, "cmd: register error: duplicate serviceId: ", serviceId);
					}

					// 注册( insert or replace )
					servicePeers[serviceId] = peer;

					// 返回成功
					return SendResponse(peer, serial, bb, "success");
				}
				else {
					// 返回错误: 收到未处理的指令
					return SendResponse_Error(peer, serial, bb, "recv unhandled cmd: ", cmd);
				}
			};
		};
	}

	// 当客户端通过 gateway 连上来后产生该事件
	virtual void AcceptSimulatePeer(std::shared_ptr<PeerType>& peer) override {
		peer->onReceiveRequest = [this, peer](int const& serial, xx::Object_s && msg)->int {
			xx::CoutN("recv request: ", msg);

			// 当前就是直接用 bb 来收发数据. 如果不是这个就报错
			auto&& bb = xx::As<xx::BBuffer>(msg);
			if (!bb) {
				return SendResponse_Error(peer, serial, bb, "msg is not bb");
			}

			// 当前就是以 cmd string + args... 作为数据格式. 先读 cmd. 再根据具体 cmd 读具体参数
			std::string cmd;
			// 读错误则返回错误
			if (auto r = bb->Read(cmd)) {
				return SendResponse_Error(peer, serial, bb, "msg read cmd error, r = ", r);
			}

			// 模拟登录流程, 收到 enter 包后, 通知相应的 game server, 令其向指定 gateway & clientId 发 open
			// enter 包：继续读出 serviceId
			if (cmd == "enter") {
				uint32_t serviceId = 0;
				// 读错误则返回错误
				if (auto r = bb->Read(serviceId)) {
					return SendResponse_Error(peer, serial, bb, "cmd: enter read serviceId error. r = ", r);
				}

				// 如果找不到 空 或 已断开 则报错退断
				auto&& iter = servicePeers.find(serviceId);
				if (iter == servicePeers.end() || !iter->second || iter->second->Disposed()) {
					return SendResponse_Error(peer, serial, bb, "cmd: enter can't find serviceId: ", serviceId);
				}

				// 定位到 gateway. 理论上讲应该必然有值( 这个函数就是从它发起调用 )
				auto gp = xx::As<xx::UvFromGatewayPeer>(peer->gatewayPeer.lock());
				assert(gp);

				// 向 service 发 enter. 捎带 gatewayId, clientId
				auto r = iter->second->SendRequest(WriteCmd(bb, "enter", gp->gatewayId, peer->id)
					, [this, peer, serial](xx::Object_s&& msg) {
					// 当前就是直接用 bb 来收发数据. 如果不是这个就报错
					auto&& bb = xx::As<xx::BBuffer>(msg);
					if (!bb) {
						SendResponse_Error(peer, serial, bb, "[enter] msg is not bb");
						return 0;
					}

					// 当前就是以 cmd string + args... 作为数据格式. 先读 cmd. 再根据具体 cmd 读具体参数
					std::string cmd;
					// 读错误则返回错误
					if (auto r = bb->Read(cmd)) {
						SendResponse_Error(peer, serial, bb, "[enter] msg read cmd error, r = ", r);
						return 0;
					}

					// 成功: 通过 sim peer 回复 success
					if (cmd == "success") {
						SendResponse(peer, serial, bb, "success");
					}
					// 出错: 转发
					else if (cmd == "error") {
						peer->SendResponse(serial, msg);
					}
					// 收到意料之外的包: 回复错误
					else {
						SendResponse_Error(peer, serial, bb, "[enter] unhandled cmd: ", cmd);
					}
					return 0;
				}, 5000);

				// 向 game service 发送失败: 给 peer 回错误信息
				if (r) {
					return SendResponse_Error(peer, serial, bb, "send enter to game peer error. r = ", r);
				}
				return 0;
			}
			else {
				// 回复未处理的指令
				return SendResponse_Error(peer, serial, bb, "unhandled cmd: ", cmd);
			}
		};
	}
};

int main() {
	Service0 s;
	s.uv.Run();
	return 0;
}
