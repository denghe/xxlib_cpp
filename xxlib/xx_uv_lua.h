#pragma once
#include "xx_uv.h"
namespace xx {
	struct UvLuaPeer : UvPeer {
		using UvPeer::UvPeer;
		Dict<int, std::pair<std::function<int(BBuffer* const& data)>, int64_t>> callbacks;

		std::function<int(BBuffer& data)> onReceivePush;
		inline int ReceivePush(BBuffer& data) noexcept { return onReceivePush ? onReceivePush(data) : 0; };
		std::function<int(int const& serial, BBuffer& data)> onReceiveRequest;
		inline int ReceiveRequest(int const& serial, BBuffer& data) noexcept { return onReceiveRequest ? onReceiveRequest(serial, data) : 0; };

		inline int SendPush(BBuffer const& data) {
			return peerBase->SendPackage(data);
		}
		inline int SendResponse(int32_t const& serial, BBuffer const& data) {
			return peerBase->SendPackage(data, serial);
		}
		inline int SendRequest(BBuffer const& data, std::function<int(BBuffer* const& data)>&& cb, uint64_t const& timeoutMS = 0) {
			if (Disposed()) return -1;
			std::pair<std::function<int(BBuffer* const& data)>, int64_t> v;
			serial = (serial + 1) & 0x7FFFFFFF;			// uint circle use
			if (timeoutMS) {
				v.second = NowSteadyEpochMS() + timeoutMS;
			}
			if (int r = peerBase->SendPackage(data, -serial)) return r;
			v.first = std::move(cb);
			callbacks[serial] = std::move(v);
			return 0;
		}

		inline virtual void Dispose(int const& flag = 1) noexcept override {
			if (!peerBase) return;
			peerBase.reset();
			timer.reset();
			for (auto&& kv : callbacks) {
				kv.value.first(nullptr);
			}
			callbacks.Clear();
			if (flag) {
				auto holder = shared_from_this();
				Disconnect();
				onDisconnect = nullptr;
				onReceivePush = nullptr;
				onReceiveRequest = nullptr;
			}
		}

	protected:
		inline virtual int HandlePack(uint8_t* const& recvBuf, uint32_t const& recvLen) noexcept override {
			auto& recvBB = uv.recvBB;
			recvBB.Reset(recvBuf, recvLen);

			int serial = 0;
			if (int r = recvBB.Read(serial)) return r;

			// 令 recvBB 只含有数据部分
			recvBB.Reset(recvBuf + recvBB.offset, recvLen - recvBB.offset);

			if (serial == 0) {
				return ReceivePush(recvBB);
			}
			else if (serial < 0) {
				return ReceiveRequest(-serial, recvBB);
			}
			else {
				auto&& idx = callbacks.Find(serial);
				if (idx == -1) return 0;
				auto a = std::move(callbacks.ValueAt(idx).first);
				callbacks.RemoveAt(idx);
				return a(&recvBB);
			}
		}

		// 因为重新定义了新的 callbacks 类型 所以需要
		// 代码直接从基类复制不修改. 
		inline virtual int Update(int64_t const& nowMS) noexcept override {
			assert(peerBase);
			if (timeoutMS && timeoutMS < nowMS) {
				Dispose(1);
				return -1;
			}

			if (int r = peerBase->Update(nowMS)) return r;

			for (auto&& iter = callbacks.begin(); iter != callbacks.end(); ++iter) {
				auto&& v = iter->value;
				if (v.second < nowMS) {
					auto a = std::move(v.first);
					iter.Remove();
					a(nullptr);
				}
			}

			return 0;
		}
	};

	using UvLuaPeer_s = std::shared_ptr<UvLuaPeer>;
}
