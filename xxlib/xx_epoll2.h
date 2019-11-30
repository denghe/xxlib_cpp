#pragma once
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


// todo: 所有函数加一波 disposed 检测?


// 内存模型为 预持有智能指针
// 从创建出来开始就放入 容器, 故不需要想办法加持
// 省掉了 lock() 能提升网络事件派发效率, 临时创建 timer 等以延迟执行一段代码等行为也变得容易
// 随之带来的问题是 要释放必须直接或间接的 Dispose( close fd, 从 epoll 移除, 从主容器移除以触发 析构 )

// 注意：
// 析构过程中无法执行 shared_from_this
// 基类析构发起的 Dispose 无法调用 派生类 override 的部分, 需谨慎
// 上层类只析构自己的数据, 到基类析构时无法访问上层类成员与 override 的函数

namespace xx::Epoll {

	/***********************************************************************************************************/
	// Item
	/***********************************************************************************************************/

	struct Context;

	struct Item : std::enable_shared_from_this<Item> {
		// 指向总容器
		Context* ep = nullptr;

		// 在对象创建成功 & 填充 & 放置到位 后会被执行, 覆盖以便做一些后期初始化工作，模拟构造函数
		inline virtual void Init() {}

		// 销毁标志
		bool disposed = false;
		inline bool Disposed() const { return disposed; }

		// flag == -1: 在析构函数中调用.  0: 不产生回调  1: 产生回调
		inline virtual void Dispose(int const& flag) {
			// 下列代码为内容示例, 方便复制小改

			// 检查 & 打标记 以避免重复执行析构逻辑
			if (disposed) return;
			disposed = true;

			// 调用派生类自己的析构部分
			Disposing(flag);

			// 这里放置 从容器移除的代码 以触发析构
		};

		// 子析构
		inline virtual void Disposing(int const& flag) {}

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
		virtual void Dispose(int const& flag) override;

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
		virtual void OnTimeout() override;

		// 从 timeoutManager 移除, call Disposing, 从容器移除( 可能触发析构 )
		virtual void Dispose(int const& flag) override;

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

		virtual int OnEpollEvent(uint32_t const& e) override;

		// 数据接收事件: 从 recv 拿数据. 默认实现为 echo
		virtual int OnReceive();

		// 用于处理超时掐线
		virtual void OnTimeout() override;

		// 断线时的处理
		inline virtual void OnDisconnect() {}

		// 触发 OnDisconnect
		virtual void Disposing(int const& flag) override;

		~TcpPeer() { this->Dispose(-1); }

		// Buf 对象塞队列并开始发送。相关信息需参考 Buf 构造函数
		int Send(xx::Buf&& data);
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
		virtual void Disposing(int const& flag) override;

		~TcpConn() { this->Dispose(-1); }
	};


	/***********************************************************************************************************/
	// UdpPeer
	/***********************************************************************************************************/

	struct UdpPeer : FDHandler {
		// 存放该 udp socket 占用的是哪个本地端口	// todo: 0 自适应还需要去提取
		int port = -1;

		// 处理数据接收
		virtual int OnEpollEvent(uint32_t const& e) override;

		// 处理数据到达事件. 默认实现为 echo. 使用 sendto 发回收到的数据.
		virtual int OnReceive(sockaddr* fromAddr, char const* const& buf, std::size_t const& len);

		// 直接封装 sendto 函数
		int SendTo(sockaddr* toAddr, char const* const& buf, std::size_t const& len);

		~UdpPeer() { this->Dispose(-1); }
	};


	/***********************************************************************************************************/
	// UdpListener
	/***********************************************************************************************************/

	struct KcpPeer;
	struct UdpListener : UdpPeer {
		// todo: 自己处理收发模拟握手 模拟 accept( 拿已创建的 KcpPeer 来分配 )
		// todo: 循环使用一组 UdpPeer, 创建逻辑 kcp 连接. 多个 UdpPeer 用于加深 epoll 事件队列深度 避免瓶颈 )
		// todo: 实现握手逻辑

		// 自增生成
		uint32_t convId = 0;

		// 握手超时时间
		int handShakeTimeoutMS = 3000;

		// 判断收到的数据内容, 模拟握手， 最后产生能 KcpPeer
		virtual int OnReceive(sockaddr* fromAddr, char const* const& buf, std::size_t const& len);

		// 覆盖并提供创建 peer 对象的实现. 返回 nullptr 表示创建失败
		virtual std::shared_ptr<KcpPeer> OnCreatePeer();

		// 覆盖并提供为 peer 绑定事件的实现. 返回非 0 表示终止 accept
		inline virtual int OnAccept(std::shared_ptr<KcpPeer>& peer) { return 0; }
	};


	/***********************************************************************************************************/
	// KcpPeer
	/***********************************************************************************************************/

	struct KcpPeer : Item, xx::TimeoutBase {
		// 用于收发数据的物理 udp peer
		std::shared_ptr<UdpListener> owner;

		// kcp 相关上下文
		ikcpcb* kcp = nullptr;
		uint32_t conv = 0;
		int64_t createMS = 0;
		uint32_t nextUpdateMS = 0;

		// 被 owner 填充
		sockaddr_in6 addr;
		std::string ip;

		// 读缓冲区内存扩容增量
		std::size_t readBufLen = 65536;

		// 读缓冲区
		xx::List<uint8_t> recv;

		// 内部函数：创建过程中确定 conv 后调用
		int InitKcp();

		// 数据接收事件: 从 recv 拿数据. 默认实现为 echo
		virtual int OnReceive();

		// 发数据( 数据可能滞留一会儿等待合并 )
		int Send(uint8_t const* const& buf, ssize_t const& dataLen);

		// 立刻发送, 停止等待
		virtual void Flush();

		// 被 ep 调用.
		// 受帧循环驱动. 帧率越高, kcp 工作效果越好. 典型的频率为 100 fps
		virtual int UpdateKcpLogic(int64_t const& nowMS);

		// 被 owner 调用.
		// 塞数据到 kcp
		int Input(uint8_t* const& recvBuf, uint32_t const& recvLen);

		// 超时自杀
		virtual void OnTimeout() override;

		virtual void Dispose(int const& flag) override;

		~KcpPeer() { this->Dispose(-1); }
	};


	/***********************************************************************************************************/
	// KcpConn
	/***********************************************************************************************************/

	// todo: 


	/***********************************************************************************************************/
	// Context
	/***********************************************************************************************************/

	struct Context {
		// fd 处理类 之 唯一持有容器. 别处引用尽量用 weak_ptr
		std::vector<std::shared_ptr<FDHandler>> fdHandlers;

		// kcp conv 值与 peer 的映射. 使用 xxDict 是为了方便遍历 kv 时 value.Dispose(1) 支持 kcp 自己从 kcps 移除
		xx::Dict<uint32_t, std::shared_ptr<KcpPeer>> kcps;

		// 带超时的握手信息字典 key: ip:port   value: conv, nowMS
		xx::Dict<std::string, std::pair<uint32_t, int64_t>> shakes;

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

		// 共享buf for kcp read 等
		std::array<char, 65536> sharedBuf;

		// maxNumFD: fd 总数
		Context(int const& maxNumFD = 65536);

		virtual ~Context();

		// 创建非阻塞 socket fd 并返回. < 0: error
		int MakeSocketFD(int const& port, int const& sockType = SOCK_STREAM); // SOCK_DGRAM

		// 添加 fd 到 epoll 监视. return !0: error
		int Ctl(int const& fd, uint32_t const& flags, int const& op = EPOLL_CTL_ADD);

		// 关闭并从 epoll 移除监视
		int CloseDel(int const& fd);

		// 进入一次 epoll wait. 可传入超时时间. 
		int Wait(int const& timeoutMS);

		// 遍历 kcp 并 Update
		void UpdateKcps();

		/********************************************************/
		// 下面是外部主要使用的函数

		// 帧逻辑可以覆盖这个函数. 返回非 0 将令 Run 退出. 
		inline virtual int Update() { return 0; }


		// 开始运行并尽量维持在指定帧率. 临时拖慢将补帧
		int Run(double const& frameRate = 60.3);

		// 创建 监听器	// todo: 支持填写ip, 支持传入复用 fd
		template<typename L = TcpListener, typename ...Args>
		std::shared_ptr<L> TcpListen(int const& port, Args&&... args);

		// 创建 连接 peer
		template<typename C = TcpConn, typename ...Args>
		std::shared_ptr<C> TcpDial(char const* const& ip, int const& port, int const& timeoutInterval, Args&&... args);

		// 创建 timer
		template<typename T = Timer, typename ...Args>
		std::shared_ptr<T> Delay(int const& interval, std::function<void(Timer* const& timer)>&& cb, Args&&...args);

		// 创建 UdpPeer
		template<typename U = UdpPeer, typename ...Args>
		std::shared_ptr<U> UdpBind(int const& port, Args&&... args);
	};

}
