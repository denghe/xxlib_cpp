#include <xx_uv_ext.h>

// 模拟了一个 0 号服务. 被 gateway & game server 连

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
			xx::CoutN("server accept: ", peer->ip);
			peer->onReceiveRequest = [this, peer](int const& serial, xx::Object_s&& msg)->int {
				// 当前就是直接用 bb 来收发数据. 如果不是这个就不认
				auto&& bb = xx::As<xx::BBuffer>(msg);
				if (!bb) return -1;

				// 当前就是以 cmd string + args 作为数据格式. 先读 cmd
				std::string cmd;
				if (auto r = bb->Read(cmd)) return r;

				// 如果是注册包. 继续读 serviceId
				if (cmd == "register") {
					uint32_t serviceId = 0;
					if (auto r = bb->Read(serviceId)) return r;

					// 如果已注册 且 连接健康 则报错退断
					auto&& iter = servicePeers.find(serviceId);
					if (iter != servicePeers.end()
						&& iter->second 
						&& !iter->second->Disposed()) {
						xx::CoutN("duplicate serviceId: ", serviceId, ", ip = ", peer->ip);
						return -3;
					}

					// 注册
					servicePeers[serviceId] = peer;

					// 回复成功. 将就 bb 构造
					bb->Clear();
					cmd = "success";
					bb->Write(cmd);
					return peer->SendResponse(serial, bb);
				}
				// todo: more cmd if

				// 回复未处理的指令. 将就 bb 构造
				bb->Clear();
				cmd = "unhandled command: " + cmd;
				bb->Write(cmd);
				return peer->SendResponse(serial, bb);
			};
		};
	}

	// 模拟登录流程, 收到 enter 包后, 通知相应的 game server, 令其向指定 gateway & clientId 发 open
	virtual void AcceptSimulatePeer(std::shared_ptr<PeerType>& sp) override {
		sp->onReceiveRequest = [this, sp](int const& serial, xx::Object_s && msg)->int {
			// 当前就是直接用 bb 来收发数据. 如果不是这个就不认
			auto&& bb = xx::As<xx::BBuffer>(msg);
			if (!bb) return -1;

			// 当前就是以 cmd string + args 作为数据格式. 先读 cmd
			std::string cmd;
			if (auto r = bb->Read(cmd)) return r;

			// enter 包：继续读出 serviceId
			if (cmd == "enter") {
				uint32_t serviceId = 0;
				if (auto r = bb->Read(serviceId)) return r;

				// 如果找不到 空 或 已断开 则报错退断
				auto&& iter = servicePeers.find(serviceId);
				if (iter == servicePeers.end()
					|| !iter->second
					|| iter->second->Disposed()) {
					xx::CoutN("can't find serviceId: ", serviceId);
					return -3;
				}

				// 定位到 gateway. 理论上讲应该必然有值( 这个函数就是从它发起调用 )
				auto gp = xx::As<xx::UvFromGatewayPeer>(sp->gatewayPeer.lock());
				assert(gp);

				// 向 service 发 enter. 捎带 gatewayId, clientId. 将就 bb 构造
				bb->Clear();
				cmd = "enter";
				bb->Write(cmd, gp->gatewayId, sp->id);
				auto r = iter->second->SendRequest(bb, [this, sp, serial](xx::Object_s&& msg) {
					// 在这个回调中，如果 sp 还活着，遇到错误都发送给 sp, return 0
					if (sp->Disposed()) return 0;

					// 当前就是直接用 bb 来收发数据. 如果不是这个就不认
					auto&& bb = xx::As<xx::BBuffer>(msg);
					if (!bb) {
						xx::MakeTo(bb);
						bb->Write(std::string("error"), -1);
						sp->SendResponse(serial, bb);
						return 0;
					}

					// 当前就是以 cmd string + args 作为数据格式. 先读 cmd
					std::string cmd;
					if (auto r = bb->Read(cmd)) {
						cmd = "error";
						bb->Write(cmd, r);
						sp->SendResponse(serial, bb);
						return 0;
					}

					// 成功: 继续给 sp 回包
					if (cmd == "success") {
						// 回复成功. 将就 bb 构造
						bb->Clear();
						cmd = "success";
						bb->Write(cmd);
						return sp->SendResponse(serial, bb);
					}
					return 0;
				}, 5000);

				// 向 game service 发送失败: 给 sp 回错误信息
				if (r) {
					bb->Clear();
					cmd = "error";
					bb->Write(cmd, -2);
					return sp->SendResponse(serial, bb);
				}

				return 0;
			}
			// todo: more cmd if

			// 回复未处理的指令. 将就 bb 构造
			bb->Clear();
			cmd = "unhandled command: " + cmd;
			bb->Write(cmd);
			return sp->SendResponse(serial, bb);
		};
	}
};

int main() {
	Service0 s;
	s.uv.Run();
	return 0;
}
