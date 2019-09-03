#include <xx_uv_ext.h>
#include <unordered_set>

struct UvGatewayPeer : xx::UvCommandPeer {
	using UvCommandPeer::UvCommandPeer;

	// ֱ��Ͷ��ԭʼ buf ����С��ת��
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

	// �������, accept ʱ���
	uint32_t clientId = 0xFFFFFFFFu;

	// ������ʵ� service peers �� id �İ�����
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

	// �ڲ�������, ���������
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

	// ��Ҫ������������Ϣ������ Dial �� onConnect ʱ���Է���Ķ�ȡ
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

	// �����ڲ����( �������������ļ������������� )
	uint32_t gatewayId = 1;

	/***********************************************************************************************/
	// client
	/***********************************************************************************************/

	// �ȴ� client �Ľ���
	std::shared_ptr<UvFromClientListener> clientListener;

	// new cp.clientId = ++clientPeerAutoId
	uint32_t clientPeerAutoId = 0;

	// key: clientId
	std::unordered_map<uint32_t, std::shared_ptr<UvFromClientPeer>> clientPeers;

	void InitClientListener() {
		// ���� listener( tcp, kcp ͬʱ֧�� )
		xx::MakeTo(clientListener, uv, "0.0.0.0", 20000, 2);

		// ��������ʱ�������� id �����ֵ� ��������Ӧ�¼��������
		clientListener->onAccept = [this](xx::UvPeer_s peer) {
			// תΪ��ȷ������
			auto&& cp = xx::As<UvFromClientPeer>(peer);

			// ���Ĭ��ת���������δ����������������
			auto&& sp_0 = serviceDialerPeers[0].second;
			if (!sp_0 || sp_0->Disposed()) {
				xx::CoutN("service 0 is not ready");
				return;
			}

			// �������� id
			cp->clientId = ++clientPeerAutoId;

			// ����ӳ���ֵ�
			clientPeers.emplace(cp->clientId, cp);

			// ע���¼�������ʱ���ֵ��Ƴ�
			cp->onDisconnect = [this, cp] {
				assert(cp->clientId);
				this->clientPeers.erase(cp->clientId);

				// Ⱥ���Ͽ�֪ͨ
				cp->serviceIds.emplace(0);	// ȷ���� 0 service Ҳ�㲥
				for (auto&& serviceId : cp->serviceIds) {
					auto&& sp = serviceDialerPeers[serviceId].second;
					if (sp && !sp->Disposed()) {
						sp->SendCommand_Disconnect(cp->clientId);
					}
				}

				xx::CoutN("client peer disconnect: ", cp->GetIP());
			};

			// ע���¼����յ�����֮����� serviceId ���ֲ���λ�� service peer ת��
			cp->onReceive = [this, cp](uint8_t* const& buf, size_t const& len)->int {
				uint32_t serviceId = 0;

				// ȡ�� serviceId
				if (len < sizeof(serviceId)) return -1;
				::memcpy(&serviceId, buf, sizeof(serviceId));

				// �жϸ÷������Ƿ��ڰ�������. �Ҳ�����Ͽ�
				if (cp->serviceIds.find(serviceId) == cp->serviceIds.end()) return -1;

				// ���Ҷ�Ӧ�� servicePeer
				auto&& sp = serviceDialerPeers[serviceId].second;

				//// ���δӳ����ѶϿ� �ͷ��ش����룬�⽫���� client peer �Ͽ�
				if (!sp || sp->Disposed()) return -2;

				// ���� 5 ��. ÿ���յ��Ϸ�������һ��
				cp->ResetTimeoutMS(5000);

				// �۸� serviceId Ϊ clientId, ת������ header ������
				::memcpy(buf, &cp->clientId, sizeof(serviceId));
				return sp->SendDirect(buf - 4, len + 4);
			};

			// ���� 5 ��. ���Ӻ� 5 �������û���յ��κ����ݣ����ӽ��Ͽ�
			cp->ResetTimeoutMS(5000);

			// ��Ĭ�Ϸ����� accept ֪ͨ
			sp_0->SendCommand_Accept(cp->clientId, cp->GetIP());

			xx::CoutN("client peer accept: ", cp->GetIP(), ", protocol = ", (cp->IsKcp() ? "kcp" : "tcp"));
		};

	}



	/***********************************************************************************************/
	// service
	/***********************************************************************************************/

	// ������ timer
	std::shared_ptr<xx::UvTimer> serviceDialTimer;

	// �������� peer �洢��һ�𣬰󶨹�ϵ
	using DialerPeer = std::pair<std::shared_ptr<UvToServiceDialer>, std::shared_ptr<UvToServicePeer>>;

	// key: serviceId
	std::unordered_map<uint32_t, DialerPeer> serviceDialerPeers;


	void InitServiceDialers() {
		// ���������� timer
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


		// todo: �����������õ� Ҫ���ӵ� service ����ϸ. ���ÿ����Ǿ���ĳ���ڲ���������ȡ
		// �������� dialer	
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

		// ������Ӧ�¼��������
		dialer->onConnect = [this, serviceId](xx::UvPeer_s peer) {
			// ���û����, ����
			if (!peer) return;

			// ��ԭ��ԭʼ���ͱ���
			auto&& sp = xx::As<UvToServicePeer>(peer);

			// ������ serviceId
			sp->serviceId = serviceId;

			// ��������
			serviceDialerPeers[serviceId].second = sp;

			// ע���¼����Ͽ�ʱ�����Ӧ peer �洢����
			sp->onDisconnect = [this, sp] {

				// �Ӵ洢���Ƴ�
				this->serviceDialerPeers[sp->serviceId].second.reset();

				// ������ client peers ��İ��������Ƴ� ���Զ��·� close. ���  ������ ���ˣ�ֱ������Ͽ�
				for (auto&& kv : clientPeers) {
					if (kv.second && !kv.second->Disposed()) {
						kv.second->SendCommand_Close(sp->serviceId);
					}
				}

				xx::CoutN("service peer disconnect: ", sp->GetIP(), ", serviceId = ", sp->serviceId);
			};

			// ע���¼����յ����͵Ĵ���
			sp->onReceive = [this, sp](uint8_t* const& buf, size_t const& len)->int {
				// ���� clientId
				uint32_t clientId = 0;
				if (len < sizeof(clientId)) return -1;
				::memcpy(&clientId, buf, sizeof(clientId));

				// ���û�ҵ� ���ѶϿ� �򷵻أ����Դ���
				auto&& iter = this->clientPeers.find(clientId);
				if (iter == this->clientPeers.end()) return 0;
				auto&& cp = iter->second;
				if (!cp || cp->Disposed()) return 0;

				// �۸� clientId Ϊ serviceId ת��( �� header )
				::memcpy(buf, &sp->serviceId, sizeof(sp->serviceId));
				(void)cp->SendDirect(buf - 4, len + 4);
				return 0;
			};

			// ע���¼����յ��ڲ�ָ��Ĵ���
			sp->onReceiveCommand = [this, sp](xx::BBuffer& bb)->int {
				// �Զ�ȡ cmd �ִ�
				std::string cmd;
				if (int r = bb.Read(cmd)) return r;

				// ���˿�. ����: clientId
				if (cmd == "open") {
					// �Զ��� clientId
					uint32_t clientId = 0;
					if (int r = bb.Read(clientId)) return r;

					// ǰ�ü��
					if (!clientId) return -1;

					// ���û�ҵ� ���ѶϿ� �򷵻أ����Դ���
					auto&& iter = this->clientPeers.find(clientId);
					if (iter == this->clientPeers.end()) return 0;
					auto&& cp = iter->second;
					if (!cp || cp->Disposed()) return 0;

					// ���������
					cp->serviceIds.emplace(sp->serviceId);

					// �·� open
					cp->SendCommand_Open(sp->serviceId);

					xx::CoutN("gateway service peer recv cmd open: clientId: ", clientId, ", serviceId = ", sp->serviceId);
					return 0;
				}


				// �ض˿�. ����: clientId
				else if (cmd == "close") {
					// �Զ��� clientId
					uint32_t clientId = 0;
					if (int r = bb.Read(clientId)) return r;

					// ǰ�ü��
					if (!clientId) return -1;

					// ���û�ҵ� ���ѶϿ� �򷵻أ����Դ���
					auto&& iter = this->clientPeers.find(clientId);
					if (iter == this->clientPeers.end()) return 0;
					auto&& cp = iter->second;
					if (!cp || cp->Disposed()) return 0;

					// �Ӱ������Ƴ�
					cp->serviceIds.erase(sp->serviceId);

					// �·� close
					cp->SendCommand_Close(sp->serviceId);

					xx::CoutN("gateway service peer recv cmd close: clientId: ", clientId, ", serviceId = ", sp->serviceId);
					return 0;
				}


				// ���������. ����: clientId, delayMS
				else if (cmd == "kick") {
					// �Զ�������
					uint32_t clientId = 0;
					int64_t delayMS = 0;
					if (int r = bb.Read(clientId, delayMS)) return r;

					// ǰ�ü��
					if (!clientId) return -1;

					// ���û�ҵ� ���ѶϿ� �򷵻أ����Դ���
					auto&& iter = this->clientPeers.find(clientId);
					if (iter == this->clientPeers.end()) return 0;
					auto&& cp = iter->second;
					if (!cp || cp->Disposed()) return 0;

					if (delayMS) {
						// �ӳٶϿ����Ƚ���¼��������������ó�ʱʱ������ʱ�� Dispose()
						cp->onDisconnect();						// ���ӳ�䲢���Ͷ���֪ͨ
						cp->onDisconnect = [cp] {};				// ����ɺ���������
						cp->onReceive = nullptr;
						cp->ResetTimeoutMS(delayMS);
					}
					else {
						// ���̶Ͽ����ӣ����� onDisconnect( �� this->clientPeers �Ƴ���������� serviceIds ��Ӧ peer �㲥�Ͽ�֪ͨ )
						cp->Dispose();
					}

					xx::CoutN("gateway service peer recv cmd kick: clientId: ", clientId);
					return 0;
				}

				else {
					return -1;
				}
			};

			// �� service �����Լ��� gatewayId
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
