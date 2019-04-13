#pragma once
#include "xx_uv.h"
namespace xx {

	template<typename BaseType>
	struct UvLuaRpcBase : BaseType {
		using BaseType::BaseType;
		using RpcFunc = std::function<int(BBuffer* const& data)>;
		std::unordered_map<int, std::pair<RpcFunc, int64_t>> callbacks;
		int serial = 0;
		std::function<int(BBuffer& data)> OnReceivePush;
		inline virtual int ReceivePush(BBuffer& data) noexcept { return OnReceivePush ? OnReceivePush(data) : 0; };
		std::function<int(int const& serial, BBuffer& data)> OnReceiveRequest;
		inline virtual int ReceiveRequest(int const& serial, BBuffer& data) noexcept { return OnReceiveRequest ? OnReceiveRequest(serial, data) : 0; };

		inline int SendPush(BBuffer const& data) {
			return this->SendPackage(data);
		}
		inline int SendResponse(int32_t const& serial, BBuffer const& data) {
			return this->SendPackage(data, serial);
		}
		inline int SendRequest(BBuffer const& data, RpcFunc&& cb, uint64_t const& timeoutMS = 0) {
			if (this->Disposed()) return -1;
			std::pair<RpcFunc, int64_t> v;
			serial = (serial + 1) & 0x7FFFFFFF;			// uint circle use
			if (timeoutMS) {
				v.second = this->uv.nowMS + timeoutMS;
			}
			if (int r = this->SendPackage(data, -serial)) return r;
			v.first = std::move(cb);
			callbacks[serial] = std::move(v);
			return 0;
		}

	protected:
		virtual int HandlePack(uint8_t* const& recvBuf, uint32_t const& recvLen) noexcept override {
			auto& recvBB = this->uv.recvBB;
			recvBB.Reset((uint8_t*)recvBuf, recvLen);

			int serial = 0;
			if (int r = recvBB.Read(serial)) return r;
			recvBB.readLengthLimit = recvLen;			// 用于 lua 创建时计算 memcpy 读取长度

			if (serial == 0) {
				return ReceivePush(recvBB);
			}
			else if (serial < 0) {
				return ReceiveRequest(-serial, recvBB);
			}
			else {
				auto&& iter = callbacks.find(serial);
				if (iter == callbacks.end()) return 0;
				auto&& a = std::move(iter->second.first);
				callbacks.erase(iter);
				return a(&recvBB);
			}
		}

		inline virtual void Update(int64_t const& nowMS) noexcept override {
			if (this->Disposed()) return;
			if (this->timeoutMS && this->timeoutMS < nowMS) {
				this->Dispose();
				return;
			}
			for (auto&& iter_ = callbacks.begin(); iter_ != callbacks.end();) {
				auto&& iter = iter_++;
				if (iter->second.second < nowMS) {
					auto&& a = std::move(iter->second.first);
					callbacks.erase(iter);
					a(nullptr);
				}
			}
		}
	};

	struct UvTcpLuaPeerBase : UvTcpBasePeer {
		using UvTcpBasePeer::UvTcpBasePeer;
	protected:
		// serial == 0: push    > 0: response    < 0: request
		inline int SendPackage(BBuffer const& data, int32_t const& serial = 0) {
			if (!uvTcp) return -1;
			auto& sendBB = uv.sendBB;
			static_assert(sizeof(uv_write_t_ex) + 4 <= 1024);
			sendBB.Reserve(1024);
			sendBB.len = sizeof(uv_write_t_ex) + 4;		// skip uv_write_t_ex + header space
			sendBB.Write(serial);
			sendBB.AddRange(data.buf, data.len);

			auto buf = sendBB.buf;						// cut buf memory for send
			auto len = sendBB.len - sizeof(uv_write_t_ex) - 4;
			sendBB.buf = nullptr;
			sendBB.len = 0;
			sendBB.cap = 0;

			return SendReqAndData(buf, (uint32_t)len);
		}
	};

	// send & recv pure data ( do not encode/decode )
	struct UvTcpLuaPeer : UvLuaRpcBase<UvTcpLuaPeerBase> {
		using BaseType = UvLuaRpcBase<UvTcpLuaPeerBase>;
		using BaseType::BaseType;

		inline virtual void Dispose(int const& flag = 1) noexcept override {
			if (!this->uvTcp) return;
			for (auto&& kv : this->callbacks) {
				kv.second.first(nullptr);
			}
			this->callbacks.clear();
			this->BaseType::Dispose(flag);
			if (flag) {
				auto holder = shared_from_this();
				this->Disconnect();
				this->OnDisconnect = nullptr;
				this->OnReceivePush = nullptr;
				this->OnReceiveRequest = nullptr;
			}
		}
		~UvTcpLuaPeer() { 
			this->Dispose(0);
		}
	};
	using UvTcpLuaPeer_s = std::shared_ptr<UvTcpLuaPeer>;
	using UvTcpLuaDialer = UvTcpDialer<UvTcpLuaPeer>;
	using UvTcpLuaDialer_s = std::shared_ptr<UvTcpLuaDialer>;

#if ENABLE_KCP
	struct UvKcpLuaPeerBase : UvKcpBasePeer {
		using UvKcpBasePeer::UvKcpBasePeer;

		// serial == 0: push    > 0: response    < 0: request
		inline int SendPackage(BBuffer const& data, int32_t const& serial = 0) {
			if (!kcp) return -1;
			auto& sendBB = uv.sendBB;
			sendBB.Resize(4);						// header len( 4 bytes )
			sendBB.Write(serial);
			sendBB.AddRange(data.buf, data.len);
			auto buf = sendBB.buf;
			auto len = sendBB.len - 4;
			buf[0] = uint8_t(len);					// fill package len
			buf[1] = uint8_t(len >> 8);
			buf[2] = uint8_t(len >> 16);
			buf[3] = uint8_t(len >> 24);
			return this->Send(buf, sendBB.len);
		}
	};

	// send & recv pure data ( do not encode/decode )
	struct UvKcpLuaPeer : UvLuaRpcBase<UvKcpLuaPeerBase> {
		using BaseType = UvLuaRpcBase<UvKcpLuaPeerBase>;
		using BaseType::BaseType;

		virtual void Dispose(int const& flag = 1) noexcept override {
			if (!kcp) return;
			RemoveFromUpdates();
			ikcp_release(kcp);
			kcp = nullptr;
			udp->Remove(conv);						// remove self from container
			udp.reset();							// unbind
			for (auto&& kv : callbacks) {
				kv.second.first(nullptr);
			}
			callbacks.clear();
			if (flag) {
				auto holder = shared_from_this();
				Disconnect();
				OnDisconnect = nullptr;
				OnReceivePush = nullptr;
				OnReceiveRequest = nullptr;
			}
		}
		~UvKcpLuaPeer() {
			this->Dispose(0);
		}
	};
	using UvKcpLuaPeer_s = std::shared_ptr<UvKcpLuaPeer>;
	using UvKcpLuaDialer = UvKcpDialer<UvKcpLuaPeer>;
	using UvKcpLuaDialer_s = std::shared_ptr<UvKcpLuaDialer>;
#endif
}
