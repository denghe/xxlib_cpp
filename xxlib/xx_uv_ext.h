#pragma once
#include "xx_uv.h"
namespace xx {

	// 主要提供给 lua 用, 发送的数据为 serial + bbuffer
	// UvPeer 基类的 SendXxxx, onReceiveXxxxx 不要用
	struct UvSerialBBufferPeer : UvPeer {
		using UvPeer::UvPeer;
		Dict<int, std::pair<std::function<int(BBuffer* const& data)>, int64_t>> callbacks;

		std::function<int(BBuffer& data)> onReceivePush;
		inline int ReceivePush(BBuffer& data) noexcept { return onReceivePush ? onReceivePush(data) : 0; };
		std::function<int(int const& serial, BBuffer& data)> onReceiveRequest;
		inline int ReceiveRequest(int const& serial, BBuffer& data) noexcept { return onReceiveRequest ? onReceiveRequest(serial, data) : 0; };

		inline int SendPush(BBuffer const& data) {
			return SendResponse(0, data);
		}
		inline int SendResponse(int32_t const& serial, BBuffer const& data) {
			auto&& bb = uv.sendBB;											// 借壳
			peerBase->SendPrepare(bb, data.len);							// 预处理
			bb.Write(serial);												// 写入 serial
			bb.AddRange(data.buf, data.len);								// 写入数据
			return peerBase->SendAfterPrepare(bb);
		}
		inline int SendRequest(BBuffer const& data, std::function<int(BBuffer* const& data)>&& cb, uint64_t const& timeoutMS = 0) {
			if (Disposed()) return -1;
			std::pair<std::function<int(BBuffer* const& data)>, int64_t> v;
			serial = (serial + 1) & 0x7FFFFFFF;			// uint circle use
			if (timeoutMS) {
				v.second = NowSteadyEpochMS() + timeoutMS;
			}
			if (int r = SendResponse(-serial, data)) return r;
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

	using UvSerialBBufferPeer_s = std::shared_ptr<UvSerialBBufferPeer>;




	// client 连 gateway 或 gateway 连 server 专用
	// UvPeer 基类的 SendXxxx, onReceiveXxxxx 不要用
	struct UvClientOrServerPeer : UvPeer {
		using UvPeer::UvPeer;

		std::function<int(uint32_t const& id, Object_s&& msg)> onReceivePush;
		std::function<int(uint32_t const& id, int const& serial, Object_s&& msg)> onReceiveRequest;

		inline int SendPush(uint32_t const& id, Object_s const& msg) {
			return SendResponse(id, 0, msg);
		}

		inline int SendResponse(uint32_t const& id, int32_t const& serial, Object_s const& msg) {
			auto&& bb = uv.sendBB;
			peerBase->SendPrepare(bb, 1024);
			bb.WriteFixed(id);
			bb.Write(serial);
			bb.WriteRoot(msg);
			return peerBase->SendAfterPrepare(bb);
		}

		inline int SendRequest(uint32_t const& id, Object_s const& msg, std::function<int(Object_s&& msg)>&& cb, uint64_t const& timeoutMS = 0) {
			if (!peerBase) return -1;
			std::pair<std::function<int(Object_s && msg)>, int64_t> v;
			serial = (serial + 1) & 0x7FFFFFFF;			// uint circle use
			if (timeoutMS) {
				v.second = NowSteadyEpochMS() + timeoutMS;
			}
			if (int r = SendResponse(id, -serial, msg)) return r;		// 这段代码从 UvPeer 复制小改，此处加上了 id 参数
			v.first = std::move(cb);
			callbacks[serial] = std::move(v);
			return 0;
		}

		// 这段代码从 UvPeer 复制
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
			auto& bb = uv.recvBB;
			bb.Reset(recvBuf, recvLen);

			uint32_t id = 0xFFFFFFFFu;
			if (int r = bb.ReadFixed(id)) return r;		// 这段代码从 UvPeer 复制小改, 增加了 id 读取

			int serial = 0;
			if (int r = bb.Read(serial)) return r;

			Object_s msg;
			if (int r = bb.ReadRoot(msg)) return r;

			if (serial == 0) {
				return onReceivePush ? onReceivePush(id, std::move(msg)) : 0;
			}
			else if (serial < 0) {
				return onReceiveRequest ? onReceiveRequest(id, -serial, std::move(msg)) : 0;
			}
			else {
				auto&& idx = callbacks.Find(serial);
				if (idx == -1) return 0;
				auto a = std::move(callbacks.ValueAt(idx).first);
				callbacks.RemoveAt(idx);
				return a(std::move(msg));
			}
		}
	};



	// gateway 监听 client 或 拨号 server 专用
	// UvPeer 基类的 SendXxxx, onReceiveXxxxx 不要用
	struct UvGatewayPeer : UvPeer {
		using UvPeer::UvPeer;

		std::function<int(uint8_t* const& buf, size_t const& len)> onReceive;

		// 这段代码从 UvPeer 复制
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
				onReceive = nullptr;		// 小改
			}
		}

	protected:

		inline virtual int HandlePack(uint8_t* const& recvBuf, uint32_t const& recvLen) noexcept override {
			// for kcp listener accept
			if (recvLen == 1 && *recvBuf == 0) {
				ip = peerBase->GetIP();
				return 0;
			}

			return onReceive ? onReceive(recvBuf, recvLen) : 0;
		}
	};
}
