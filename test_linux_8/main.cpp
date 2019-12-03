#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/uio.h>
#include <sys/signalfd.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "ikcp.h"
#include "xx_bbuffer.h"
#include "xx_dict.h"
#include "xx_queue.h"
#include "xx_buf.h"
#include "xx_buf_queue.h"
#include "xx_timeout.h"
#include "xx_itempool.h"

namespace xx {
	struct Context;

	struct Item {
		// 指向 context
		Context* ep = nullptr;
		virtual ~Item() {}
	};

	struct FDItem : Item {
		// linux 系统文件描述符. 同时也是 ep->fdHandlers 的下标
		int fd = -1;

		// epoll fd 事件处理. return 非 0 表示自杀
		virtual int OnEpollEvent(uint32_t const& e) = 0;

		// 关闭 fd 啥的
		virtual ~FDItem();
	};

	// 针对 FDItem 的 安全弱引用伪指针. 几个操作符每次都会检查是否失效. 失效可以被 try 到。
	template<typename T, typename ENABLED = std::enable_if_t<std::is_base_of_v<FDItem, T>>>
	struct FDItem_r {
		int index = -1;
		uint64_t version = 0;

		FDItem_r(T* const& ptr);

		FDItem_r() = default;
		FDItem_r(FDItem_r const&) = default;
		FDItem_r& operator=(FDItem_r const&) = default;

		operator bool() const;
		T* operator->() const;
		T& operator*() const;
	};

	struct TcpListener;
	struct TcpPeer : FDItem, xx::TimeoutBase {
		// ep 本身就是 timeout 管理器
		virtual TimeoutManager* GetTimeoutManager() override;

		// ip
		std::string ip;

		// 收数据用堆积容器
		xx::List<uint8_t> recv;

		// 是否正在发送( 是：不管 sendQueue 空不空，都不能 write, 只能塞 sendQueue )
		bool writing = false;

		// 待发送队列
		xx::BufQueue sendQueue;

		// 每 fd 每一次可写, 写入的长度限制( 希望能实现当大量数据下发时各个 socket 公平的占用带宽 )
		std::size_t sendLenPerFrame = 65536;

		// 读缓冲区内存扩容增量
		std::size_t readBufLen = 65536;

		virtual int OnEpollEvent(uint32_t const& e) override;

		// 数据接收事件: 从 recv 拿数据. 默认实现为 echo
		virtual int OnReceive();

		// 用于处理超时掐线
		virtual void OnTimeout() override;

		// 断线时的处理
		inline virtual void OnDisconnect() {}

		// SetTimeout(0)
		~TcpPeer();

		// Buf 对象塞队列并开始发送。相关信息需参考 Buf 构造函数
		int Send(xx::Buf&& data);
		int Flush();

	protected:
		int Write();
		int Read();
	};



	//struct TcpListener : FDItem {
	//	// 覆盖并提供创建 peer 对象的实现. 返回 nullptr 表示创建失败
	//	virtual std::shared_ptr<TcpPeer> OnCreatePeer();

	//	// 覆盖并提供为 peer 绑定事件的实现. 返回非 0 表示终止 accept
	//	inline virtual int OnAccept(std::shared_ptr<TcpPeer>& peer) { return 0; }

	//	// 调用 accept
	//	virtual int OnEpollEvent(uint32_t const& e) override;

	//	~TcpListener() { this->Dispose(-1); }

	//protected:
	//	// return fd. <0: error. 0: empty (EAGAIN / EWOULDBLOCK), > 0: fd
	//	int Accept(int const& listenFD);
	//};

	struct Context : TimeoutManager {
		// fd 处理类 之 唯一持有容器. 原则上逻辑全部用弱引用. int64_t: version
		inline static std::array<std::pair<std::unique_ptr<FDItem>, int64_t>, 40000> fdHandlers;

		// 存储的 epoll fd
		int efd = -1;
	};

	inline FDItem::~FDItem() {
		if (fd != -1) {
			assert(ep);
			(void)epoll_ctl(ep->efd, EPOLL_CTL_DEL, fd, nullptr);
			close(fd);
			fd = -1;
		}
	}

	template<typename T, typename ENABLED>
	inline FDItem_r<T, ENABLED>::FDItem_r(T* const& ptr) {
	}

	template<typename T, typename ENABLED>
	inline FDItem_r<T, ENABLED>::operator bool() const {
		return !version && Context::fdHandlers[index].second == version;
	}

	template<typename T, typename ENABLED>
	inline T* FDItem_r<T, ENABLED>::operator->() const {
		if (!operator bool()) throw - 1;					// 空指针
		return Context::fdHandlers[index].first.get();
	}

	template<typename T, typename ENABLED>
	inline T& FDItem_r<T, ENABLED>::operator*() const {
		return *operator->();
	}




	//inline TimeoutManager* TcpPeer::GetTimeoutManager() {
	//	return ep;
	//}

	//inline void TcpPeer::~TcpPeer() {
	//	SetTimeout(0);
	//}

	//inline int TcpPeer::Send(xx::Buf&& data) {
	//	sendQueue.Push(std::move(data));
	//	return !writing ? Write() : 0;
	//}

	//inline int TcpPeer::Flush() {
	//	return !writing ? Write() : 0;
	//}

	//inline int TcpPeer::Write() {
	//	// 如果没有待发送数据，停止监控 EPOLLOUT 并退出
	//	if (!sendQueue.bytes) return ep->Ctl(fd, EPOLLIN, EPOLL_CTL_MOD);

	//	// 前置准备
	//	std::array<iovec, UIO_MAXIOV> vs;					// buf + len 数组指针
	//	int vsLen = 0;										// 数组长度
	//	auto bufLen = sendLenPerFrame;						// 计划发送字节数

	//	// 填充 vs, vsLen, bufLen 并返回预期 offset. 每次只发送 bufLen 长度
	//	auto&& offset = sendQueue.Fill(vs, vsLen, bufLen);

	//	// 返回值为 实际发出的字节数
	//	auto&& sentLen = writev(fd, vs.data(), vsLen);

	//	// 已断开
	//	if (sentLen == 0) return -2;

	//	// 繁忙 或 出错
	//	else if (sentLen == -1) {
	//		if (errno == EAGAIN) goto LabEnd;
	//		else return -3;
	//	}

	//	// 完整发送
	//	else if ((std::size_t)sentLen == bufLen) {
	//		// 快速弹出已发送数据
	//		sendQueue.Pop(vsLen, offset, bufLen);

	//		// 这次就写这么多了. 直接返回. 下次继续 Write
	//		return 0;
	//	}

	//	// 发送了一部分
	//	else {
	//		// 弹出已发送数据
	//		sendQueue.Pop(sentLen);
	//	}

	//LabEnd:
	//	// 标记为不可写
	//	writing = true;

	//	// 开启对可写状态的监控, 直到队列变空再关闭监控
	//	return ep->Ctl(fd, EPOLLIN | EPOLLOUT, EPOLL_CTL_MOD);
	//}

	//inline int TcpPeer::Read() {
	//	// 如果接收缓存没容量就扩容( 通常发生在首次使用时 )
	//	if (!recv.cap) {
	//		recv.Reserve(readBufLen);
	//	}

	//	// 如果数据长度 == buf限长 就自杀( 未处理数据累计太多? )
	//	if (recv.len == recv.cap) return -1;

	//	// 通过 fd 从系统网络缓冲区读取数据. 追加填充到 recv 后面区域. 返回填充长度. <= 0 则认为失败 自杀
	//	auto&& len = read(fd, recv.buf + recv.len, recv.cap - recv.len);
	//	if (len <= 0) return -2;
	//	recv.len += len;

	//	// 调用用户数据处理函数
	//	return OnReceive();
	//}

	//inline int TcpPeer::OnEpollEvent(uint32_t const& e) {
	//	// read
	//	if (e & EPOLLIN) {
	//		if (int r = Read()) return r;
	//	}
	//	// write
	//	if (e & EPOLLOUT) {
	//		// 设置为可写状态
	//		writing = false;
	//		if (int r = Write()) return r;
	//	}
	//	return 0;
	//}

	//inline int TcpPeer::OnReceive() {
	//	// 默认实现为 echo. 仅供测试. 随意使用 write 可能导致待发队列中的数据被分割
	//	auto&& r = write(fd, recv.buf, recv.len) == (ssize_t)recv.len ? 0 : -1;
	//	recv.Clear();
	//	return r;
	//}

	//inline void TcpPeer::OnTimeout() {
	//	Dispose(1);
	//}



	// todo: more func forward for easy use
};

int main(int argc, char** argv) {

	return 0;
}


// 下面代码展示一种 try 空指针的方式
//
//template<typename T>
//struct Ptr {
//	T* ptr = nullptr;
//	int lineNumber = -1;
//	T* operator->() {
//		if (!ptr) throw lineNumber;
//		return ptr;
//	}
//	void Clear(int const& lineNumber) {
//		if (ptr) {
//			delete ptr;
//			ptr = nullptr;
//			this->lineNumber = lineNumber;
//		}
//	}
//	void Reset(T* const& ptr, int const& lineNumber) {
//		Clear(lineNumber);
//		this->ptr = ptr;
//	}
//};
//
//#define PtrReset(self, ptr) self.Reset(nullptr, __LINE__);
//#define PtrClear(self) self.Clear(__LINE__);
//
//struct Foo {
//	int n = 0;
//	bool disposed = false;
//};
//
//int main(int argc, char** argv) {
//
//	Foo* f = new Foo;
//	f->n = 123;
//	Ptr<Foo> p;
//	p.ptr = f;
//	try {
//		xx::CoutN(p->n);
//		PtrClear(p);
//		xx::CoutN(p->n);
//	}
//	catch (int const& lineNumber) {
//		std::cout << lineNumber << std::endl;
//	}
//	catch (std::exception const& e) {
//		std::cout << e.what() << std::endl;
//	}
//}