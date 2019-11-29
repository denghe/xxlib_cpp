#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/uio.h>
#include <errno.h>
#include <signal.h>
#include <sys/signalfd.h>
#include <netinet/tcp.h>

#include "xx_bbuffer.h"
#include "xx_queue.h"
#include "xx_buf.h"
#include "xx_buf_queue.h"
#include "xx_timeout.h"

// todo: 加一波 noexcept, const 啥的

// 内存模型为 智能指针 从创建出来开始就放入 容器, 并非弱引用, 故不需要想办法加持, 也能提升网络事件派发效率( 省掉了 lock() )
// Dispose 主要作用：close fd, 从 epoll 移除, 从主容器移除
// 注意：
// 析构调用 Dispose 的情况可能是 对象创建到一半 发生异常, 并且此时无法调用 派生类 override 的部分, 故需要谨慎判断. 且因为一定不是位于主容器，故也不能执行移除相关代码
// Disposing 主供派生类 从非析构函数源头执行 派生层面增加的析构内容
// 注意2：
// 从析构函数发起的调用，无法执行 shared_from_this

namespace xx::Epoll {

	/***********************************************************************************************************/
	// Item
	/***********************************************************************************************************/

	struct Context;

	struct Item : std::enable_shared_from_this<Item> {
		// 指向总容器
		Context* ep = nullptr;

		// 销毁标志
		bool disposed = false;
		inline bool Disposed() const noexcept {
			return disposed;
		}

		// flag == -1: 在析构函数中调用.  0: 不产生回调  1: 产生回调
		inline virtual void Dispose(int const& flag) noexcept {
			// 下列代码为内容示例, 方便复制小改

			// 检查 & 打标记 以避免重复执行析构逻辑
			if (disposed) return;
			disposed = true;

			// 调用派生类自己的析构部分
			Disposing(flag);

			// 这里放置 从容器移除的代码 以触发析构
		};

		// 子析构
		inline virtual void Disposing(int const& flag) noexcept {}

		// 每层非虚派生类析构都要写 this->Dispose(-1);
		virtual ~Item() { this->Dispose(-1); }
	};


	/***********************************************************************************************************/
	// FDHandler
	/***********************************************************************************************************/

	struct FDHandler : Item {
		// linux 系统文件描述符. 同时也是 ep->fdHandlers 的下标
		int fd = -1;

		// epoll fd 事件处理. return 非 0 表示自杀
		virtual int OnEpollEvent(uint32_t const& e) = 0;

		// 关 fd, 从 epoll 移除, call Disposing, 从容器移除( 可能触发析构 )
		virtual void Dispose(int const& flag) noexcept override;

		virtual ~FDHandler() { this->Dispose(-1); }
	};


	/***********************************************************************************************************/
	// Timer
	/***********************************************************************************************************/

	struct Timer : Item, xx::TimeoutBase {
		// ep->timers 的下标
		int indexAtContainer = -1;

		// 时间到达时触发. 如果想实现 repeat 效果, 就在函数返回前 自己 timer->SetTimeout
		std::function<void(Timer* const& timer)> onFire;

		// 负责触发 onFire
		virtual void OnTimeout() noexcept override;

		// 从 timeoutManager 移除, call Disposing, 从容器移除( 可能触发析构 )
		virtual void Dispose(int const& flag) noexcept override;

		~Timer() { this->Dispose(-1); }
	};


	/***********************************************************************************************************/
	// TcpPeer
	/***********************************************************************************************************/

	struct TcpListener;
	struct TcpPeer : FDHandler, xx::TimeoutBase {
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

		// writev 函数 (buf + len) 数组 参数允许的最大数组长度
		static const std::size_t maxNumIovecs = 1024;

		virtual int OnEpollEvent(uint32_t const& e) override;

		// 数据接收事件: 从 recv 拿数据
		virtual int OnReceive();

		// 用于处理超时掐线
		virtual void OnTimeout() override;

		// 断线时的处理
		inline virtual void OnDisconnect() {}

		// 触发 OnDisconnect
		virtual void Disposing(int const& flag) noexcept override;

		~TcpPeer() { this->Dispose(-1); }

		int Send(xx::Buf&& eb);
		int Flush();

	protected:
		int Write();
		int Read();
	};


	/***********************************************************************************************************/
	// TcpListener
	/***********************************************************************************************************/

	struct TcpListener : FDHandler {
		// 覆盖并提供创建 peer 对象的实现. 返回 nullptr 表示创建失败
		virtual std::shared_ptr<TcpPeer> OnCreatePeer();

		// 覆盖并提供为 peer 绑定事件的实现. 返回非 0 表示终止 accept
		inline virtual int OnAccept(std::shared_ptr<TcpPeer>& peer) { return 0; }

		// 调用 accept
		virtual int OnEpollEvent(uint32_t const& e) override;

		~TcpListener() { this->Dispose(-1); }

	protected:
		// return fd. <0: error. 0: empty (EAGAIN / EWOULDBLOCK), > 0: fd
		int Accept(int const& listenFD);
	};


	/***********************************************************************************************************/
	// TcpConn
	/***********************************************************************************************************/

	struct TcpConn : TcpPeer {
		// 是否连接成功
		bool connected = false;

		// 成功, 超时或连接错误 都将触发该函数. 进一步判断 connected 可知状态
		inline virtual void OnConnect() {}

		// 通过 connected 来路由两套事件逻辑
		virtual int OnEpollEvent(uint32_t const& e) override;

		// 触发 OnConnect
		virtual void Disposing(int const& flag) noexcept override;

		~TcpConn() { this->Dispose(-1); }
	};


	/***********************************************************************************************************/
	// UdpPeer
	/***********************************************************************************************************/

	struct UdpPeer : FDHandler {
		// todo: 提供 udp 基础收发功能
		~UdpPeer() { this->Dispose(-1); }
	};


	/***********************************************************************************************************/
	// UdpListener
	/***********************************************************************************************************/

	struct UdpListener : UdpPeer {
		// todo: 自己处理收发模拟握手 模拟 accept( 拿已创建的 KcpPeer 来分配 )
		// 循环使用一组 UdpPeer, 创建逻辑 kcp 连接. 多个 UdpPeer 用于加深 epoll 事件队列深度 避免瓶颈 )
	};


	/***********************************************************************************************************/
	// UdpDialer
	/***********************************************************************************************************/

	struct UdpDialer : UdpPeer {
	};


	/***********************************************************************************************************/
	// KcpPeer
	/***********************************************************************************************************/

	struct KcpPeer : UdpPeer {
	};


	/***********************************************************************************************************/
	// Context
	/***********************************************************************************************************/

	struct Context {
		// fd 处理类 之 唯一持有容器. 别处引用尽量用 weak_ptr
		std::vector<std::shared_ptr<FDHandler>> fdHandlers;

		// timer 唯一持有容器. 别处引用尽量用 weak_ptr
		std::vector<std::shared_ptr<Timer>> timers;

		// epoll_wait 事件存储
		std::array<epoll_event, 4096> events;

		// 存储的 epoll fd
		int efd = -1;

		// 对于一些返回值非 int 的函数, 具体错误码将存放于此
		int lastErrorNumber = 0;

		// 超时管理器
		xx::TimeoutManager timeoutManager;

		// maxNumFD: fd 总数
		Context(int const& maxNumFD = 65536);

		~Context();

		// 创建监听用 tcp fd 并返回. < 0: error
		static int MakeListenFD(int const& port);

		// 添加 fd 到 epoll 监视. return !0: error
		int Ctl(int const& fd, uint32_t const& flags, int const& op = EPOLL_CTL_ADD);

		// 关闭并从 epoll 移除监视
		int CloseDel(int const& fd);

		// 进入一次 epoll wait. 可传入超时时间. 
		int Wait(int const& timeoutMS);


		/********************************************************/
		// 下面是外部主要使用的函数

		// 帧逻辑可以覆盖这个函数. 返回非 0 将令 Run 退出. 
		inline virtual int Update() { return 0; }

		// 开始运行并尽量维持在指定帧率. 临时拖慢将补帧
		int Run(double const& frameRate = 60.3);

		// 创建 监听器
		template<typename L = TcpListener, typename ...Args>
		inline std::shared_ptr<L> TcpListen(int const& port, Args&&... args);

		// 创建 连接 peer
		template<typename C = TcpConn, typename ...Args>
		inline std::shared_ptr<C> TcpDial(char const* const& ip, int const& port, int const& timeoutInterval, Args&&... args);

		// 创建 timer
		template<typename T = Timer, typename ...Args>
		inline std::shared_ptr<T> Delay(int const& interval, std::function<void(Timer* const& timer)>&& cb, Args&&...args);
	};

}
