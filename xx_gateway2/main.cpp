#include <xx_uv_ext.h>
#include <unordered_set>

struct UvFrontPear : xx::UvCommandPeer {
	using UvCommandPeer::UvCommandPeer;

	// ֱ��Ͷ��ԭʼ buf ����С��ת��
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

	// ��Ҫ������������Ϣ������ Dial �� onConnect ʱ���Է���Ķ�ȡ
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

	// �ȴ� client �Ľ���
	std::shared_ptr<UvFromClientListener> clientListener;

	// new cp.clientId = ++clientPeerAutoId
	uint32_t clientPeerAutoId = 0;

	// key: clientId
	std::unordered_map<uint32_t, std::shared_ptr<UvFromClientPeer>> clientPeers;

	// ��ʱ������ɶ
	std::vector<uint8_t> tmpBufPlaceHolder;
	uint8_t* tmpBuf = nullptr;
	std::size_t tmpBufLen = 0;

	void InitVars() {
		tmpBufPlaceHolder.resize(uv.maxPackageLength + 8);	// 8: len + clientId
		tmpBuf = tmpBufPlaceHolder.data();
		tmpBufLen = tmpBufPlaceHolder.size();
	}

	void InitClientListener() {
		// ���� listener( tcp, kcp ͬʱ֧�� )
		xx::MakeTo(clientListener, uv, "0.0.0.0", 20000, 2);

		// ��������ʱ�������� id �����ֵ� ��������Ӧ�¼��������
		clientListener->onAccept = [this](xx::UvPeer_s peer) {
			if (!rearPeerAlive()) {
				xx::CoutN("rear is not ready");
				return;
			}

			// תΪ��ȷ������
			auto&& cp = xx::As<UvFromClientPeer>(peer);

			// �������� id
			cp->clientId = ++clientPeerAutoId;

			// ����ӳ���ֵ�
			clientPeers.emplace(cp->clientId, cp);

			// ע���¼�������ʱ���ֵ��Ƴ�
			cp->onDisconnect = [this, cp] {
				assert(cp->clientId);
				this->clientPeers.erase(cp->clientId);

				// ���Ͽ�֪ͨ( rear �Ǳ�Ҳ�� serviceIds. �յ���ָ��� ����Ⱥ���Ͽ�֪ͨ )
				if (rearPeerAlive()) {
					rearPeer->SendCommand_Disconnect(cp->clientId);
				}

				xx::CoutN("client peer disconnect: ", cp->GetIP());
			};

			// ע���¼����յ�����֮��ת���� rear
			cp->onReceive = [this, cp](uint8_t* const& buf, std::size_t const& len)->int {
				assert(len + 8 <= tmpBufLen);

				// ����� rear �����Ӳ�������Ͽ�
				if (!rearPeerAlive()) return -2;

				// ȡ�� serviceId
				uint32_t serviceId = 0;
				if (len < sizeof(serviceId)) return -1;
				::memcpy(&serviceId, buf, sizeof(serviceId));

				// �жϸ÷������Ƿ��ڰ�������. �Ҳ�����Ͽ�
				if (cp->serviceIds.find(serviceId) == cp->serviceIds.end()) return -1;

				// ��������( buf �������� 8 �ֽ�Ԥ�� )
				*(uint32_t*)(buf - 8) = (uint32_t)(len + 4);

				// ��� senderId
				*(uint32_t*)(buf - 4) = cp->clientId;

				// ���� 5 ��. ÿ���յ��Ϸ�������һ��
				cp->ResetTimeoutMS(5000);

				// ����. �ܳ���Ϊ sizeof(header) + sizeof(senderId) + len
				return rearPeer->SendDirect(buf - 8, len + 8);
			};

			// ���� 5 ��. ���Ӻ� 5 �������û���յ��κ����ݣ����ӽ��Ͽ�
			cp->ResetTimeoutMS(5000);

			// �� rear ���� accept ֪ͨ( rear ������Ĭ�Ϸ���ת�� )
			rearPeer->SendCommand_Accept(cp->clientId, cp->GetIP());

			xx::CoutN("client peer accept: ", cp->GetIP(), ", protocol = ", (cp->IsKcp() ? "kcp" : "tcp"));
		};
	}



	/***********************************************************************************************/
	// real ���
	/***********************************************************************************************/

	std::shared_ptr<xx::UvTimer> rearDialerTimer;
	std::shared_ptr<UvToRearDialer> rearDialer;
	std::shared_ptr<UvToRearPeer> rearPeer;
	bool rearPeerAlive() {
		return rearPeer && !rearPeer->Disposed();
	}

	void InitServiceDialers() {
		// ���������� timer
		xx::MakeTo(rearDialerTimer, uv, 500, 500, [this] {
			if (!rearPeerAlive()) {
				if (!rearDialer->Busy()) {
					rearDialer->Dial();
					xx::CoutN("rear rearDialer dial...");
				}
			}
		});

		// todo: �����������õ� Ҫ���ӵ� rear ����ϸ. ���ÿ����Ǿ���ĳ���ڲ���������ȡ
		TryCreateRearDialer("127.0.0.1", 22220);
	}

	int TryCreateRearDialer(std::string const& ip, int const& port) {
		if (rearPeer) return -1;

		xx::TryMakeTo(rearDialer, uv);
		if (!rearDialer) return -2;

		rearDialer->ip = ip;
		rearDialer->port = port;

		// ������Ӧ�¼��������
		rearDialer->onConnect = [this](xx::UvPeer_s peer) {
			// ���û����, ����
			if (!peer) return;

			// ��ԭ��ԭʼ���ͱ���
			rearPeer = xx::As<UvToRearPeer>(peer);

			// ע���¼���rear �Ͽ�ʱ����洢�Լ��Ͽ����� client
			rearPeer->onDisconnect = [this] {
				this->rearPeer.reset();
				clientPeers.clear();
				xx::CoutN("rear peer disconnect.");
			};

			// ע���¼����յ����͵Ĵ���
			rearPeer->onReceive = [this](uint8_t* const& buf, std::size_t const& len)->int {
				// ���� serviceId
				uint32_t serviceId = 0;
				if (len < sizeof(serviceId)) return -1;
				::memcpy(&serviceId, buf, sizeof(serviceId));

				// ���� clientId
				uint32_t clientId = 0;
				if (len < sizeof(clientId)) return -1;
				::memcpy(&clientId, buf + 4, sizeof(clientId));

				// ��� ��Ӧ client û�ҵ� ���ѶϿ� �򷵻أ����Դ���
				auto&& iter = this->clientPeers.find(clientId);
				if (iter == this->clientPeers.end()) return 0;
				auto&& cp = iter->second;
				if (!cp || cp->Disposed()) return 0;

				// �۸� serviceId Ϊ ����
				auto dataLen = (uint32_t)(len - 4);
				::memcpy(buf, &dataLen, sizeof(dataLen));

				// �۸� clientId Ϊ serviceId
				::memcpy(buf + 4, &serviceId, sizeof(serviceId));

				// ����
				(void)cp->SendDirect(buf, len);
				return 0;
			};

			// ע���¼����յ��ڲ�ָ��Ĵ���
			rearPeer->onReceiveCommand = [this](xx::BBuffer& bb)->int {
				// �Զ�ȡ cmd �ִ�
				std::string cmd;
				if (int r = bb.Read(cmd)) return r;

				// ���˿�. ����: clientId, serviceId
				if (cmd == "open") {
					// �Զ��� clientId, serviceId
					uint32_t clientId = 0, serviceId = 0;
					if (int r = bb.Read(clientId, serviceId)) return r;

					// ���û�ҵ� ���ѶϿ� �򷵻أ����Դ���
					auto&& iter = this->clientPeers.find(clientId);
					if (iter == this->clientPeers.end()) return 0;
					auto&& cp = iter->second;
					if (!cp || cp->Disposed()) return 0;

					// ���������
					cp->serviceIds.emplace(serviceId);

					// �·� open
					cp->SendCommand_Open(serviceId);

					xx::CoutN("gateway service peer recv cmd open: clientId: ", clientId, ", serviceId = ", serviceId);
					return 0;
				}


				// �ض˿�. ����: clientId, serviceId
				else if (cmd == "close") {
					// �Զ��� clientId, serviceId
					uint32_t clientId = 0, serviceId = 0;
					if (int r = bb.Read(clientId, serviceId)) return r;

					// ���û�ҵ� ���ѶϿ� �򷵻أ����Դ���
					auto&& iter = this->clientPeers.find(clientId);
					if (iter == this->clientPeers.end()) return 0;
					auto&& cp = iter->second;
					if (!cp || cp->Disposed()) return 0;

					// �Ӱ������Ƴ�
					cp->serviceIds.erase(serviceId);

					// �·� close
					cp->SendCommand_Close(serviceId);

					xx::CoutN("gateway service peer recv cmd close: clientId: ", clientId, ", serviceId = ", serviceId);
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
