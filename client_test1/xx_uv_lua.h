#pragma once
#include "xx_uv.h"
namespace xx {
	struct UvTcpLuaPeerBase : UvTcpPeerBase {
		using UvTcpPeerBase::UvTcpPeerBase;
	protected:
		// serial == 0: push    > 0: response    < 0: request
		inline int LuaSendPackage(BBuffer const& data, int32_t const& serial = 0) {
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

	struct UvKcpLuaPeerBase : UvKcpPeerBase {
		using UvKcpPeerBase::UvKcpPeerBase;
	protected:
		// serial == 0: push    > 0: response    < 0: request
		inline int LuaSendPackage(BBuffer const& data, int32_t const& serial = 0) {
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

	template<typename BaseType>
	struct UvLuaRpcBase : BaseType {
		using RpcFunc = std::function<int(BBuffer* const& data)>;
		int serial = 0;
		Dict<int, std::pair<RpcFunc, int64_t>> callbacks;
		UvTimer_s timer;

		std::function<int(BBuffer& data)> onReceivePush;
		inline int LuaReceivePush(BBuffer& data) noexcept { return onReceivePush ? onReceivePush(data) : 0; };
		std::function<int(int const& serial, BBuffer& data)> onReceiveRequest;
		inline int LuaReceiveRequest(int const& serial, BBuffer& data) noexcept { return onReceiveRequest ? onReceiveRequest(serial, data) : 0; };

		UvLuaRpcBase(Uv& uv) : BaseType(uv) {
			MakeTo(timer, uv, 10, 10, [this] {
				this->Update(NowSteadyEpochMS());
			});
		}

		inline int LuaSendPush(BBuffer const& data) {
			return this->LuaSendPackage(data);
		}
		inline int LuaSendResponse(int32_t const& serial, BBuffer const& data) {
			return this->LuaSendPackage(data, serial);
		}
		inline int LuaSendRequest(BBuffer const& data, RpcFunc&& cb, uint64_t const& timeoutMS = 0) {
			if (this->Disposed()) return -1;
			std::pair<RpcFunc, int64_t> v;
			serial = (serial + 1) & 0x7FFFFFFF;			// uint circle use
			if (timeoutMS) {
				v.second = this->uv.nowMS + timeoutMS;
			}
			if (int r = this->LuaSendPackage(data, -serial)) return r;
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
				return LuaReceivePush(recvBB);
			}
			else if (serial < 0) {
				return LuaReceiveRequest(-serial, recvBB);
			}
			else {
				auto&& idx = callbacks.Find(serial);
				if (idx == -1) return 0;
				auto a = std::move(callbacks.ValueAt(idx).first);
				callbacks.RemoveAt(idx);
				return a(&recvBB);
			}
		}

		inline virtual void Update(int64_t const& nowMS) noexcept override {
			assert(!this->Disposed());
			this->BaseType::Update(nowMS);

			if (this->timeoutMS && this->timeoutMS < nowMS) {
				this->Dispose(1);
				return;
			}

			for (auto&& iter = this->callbacks.begin(); iter != this->callbacks.end(); ++iter) {
				auto&& v = iter->value;
				if (v.second < nowMS) {
					auto a = std::move(v.first);
					iter.Remove();
					a(nullptr);
				}
			}
		}

		// todo: impl all unused IUvPeer interfaces
	};


	// send & recv pure data ( do not encode/decode )
	struct UvTcpLuaPeer : UvLuaRpcBase<UvTcpLuaPeerBase> {
		using BaseType = UvLuaRpcBase<UvTcpLuaPeerBase>;
		using BaseType::BaseType;

		~UvTcpLuaPeer() { 
			this->Dispose(0);
		}
		inline virtual void Dispose(int const& flag = 1) noexcept override {
			if (this->Disposed()) return;
			Uv::HandleCloseAndFree(this->uvTcp);
			for (auto&& kv : this->callbacks) {
				kv.value.first(nullptr);
			}
			this->callbacks.Clear();
			if (flag) {
				auto holder = shared_from_this();
				this->timer.reset();
				this->Disconnect();
				this->onDisconnect = nullptr;
				this->onReceivePush = nullptr;
				this->onReceiveRequest = nullptr;
			}
		}
	};
	using UvTcpLuaPeer_s = std::shared_ptr<UvTcpLuaPeer>;
	using UvTcpLuaDialer = UvTcpDialer<UvTcpLuaPeer>;
	using UvTcpLuaDialer_s = std::shared_ptr<UvTcpLuaDialer>;


	// send & recv pure data ( do not encode/decode )
	struct UvKcpLuaPeer : UvLuaRpcBase<UvKcpLuaPeerBase> {
		using BaseType = UvLuaRpcBase<UvKcpLuaPeerBase>;
		using BaseType::BaseType;

		~UvKcpLuaPeer() {
			this->Dispose(0);
		}
		virtual void Dispose(int const& flag = 1) noexcept override {
			if (this->Disposed()) return;
			this->timer.reset();
			ikcp_release(kcp);
			this->kcp = nullptr;
			this->udp->Remove(conv);					// remove self from container
			this->udp.reset();							// unbind
			for (auto&& kv : this->callbacks) {
				kv.value.first(nullptr);
			}
			this->callbacks.Clear();
			if (flag) {
				auto holder = shared_from_this();
				this->Disconnect();
				this->onDisconnect = nullptr;
				this->onReceivePush = nullptr;
				this->onReceiveRequest = nullptr;
			}
		}
	};
	using UvKcpLuaPeer_s = std::shared_ptr<UvKcpLuaPeer>;
	using UvKcpLuaDialer = UvKcpDialer<UvKcpLuaPeer>;
	using UvKcpLuaDialer_s = std::shared_ptr<UvKcpLuaDialer>;
}
