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
#include "xx_itempool.h"





// 注意：

// 所有调用 虚函数 的地方是否有 alive 检测? 需检测上层函数是否已 call Dispose

// 析构过程中无法执行 shared_from_this
// 基类析构发起的 Dispose 无法调用 派生类 override 的部分, 需谨慎
// 上层类只析构自己的数据, 到基类析构时无法访问上层类成员与 override 的函数


namespace xx::Epoll {

	/***********************************************************************************************************/
	// Item
	/***********************************************************************************************************/

	struct Context;
	struct Item {
		// 指向总容器
		Context* ep = nullptr;

		// item 所在容器下标
		int indexAtContainer = -1;

		// 从 ep->items 移除自己, 进而触发析构
		void Dispose();

		virtual ~Item() {}
	};
	using Item_u = std::unique_ptr<Item>;

	/***********************************************************************************************************/
	// FDItem
	/***********************************************************************************************************/

	struct FDItem : Item {
		// linux 系统文件描述符
		int fd = -1;

		// epoll fd 事件处理. 有问题自己直接 Dispose 自杀
		virtual void OnEpollEvent(uint32_t const& e) = 0;

		// 关闭 fd 啥的
		virtual ~FDItem();
	};
	using FDItem_u = std::unique_ptr<FDItem>;

	/***********************************************************************************************************/
	// Ref
	/***********************************************************************************************************/

	// 针对 Item 的 弱引用伪指针. 几个操作符每次都会检查是否失效. 失效可以被 try 到。
	template<typename T>
	struct Ref {
		ItemPool<Item_u>* items = nullptr;
		int index = -1;
		int64_t version = 0;

		// 会从 ptr 中提取 ep & indexAtContainer. 故需要保证这些值有效
		Ref(T* const& ptr);
		Ref(std::unique_ptr<T> const& ptr);

		Ref() = default;
		Ref(Ref const&) = default;
		Ref& operator=(Ref const&) = default;

		operator bool() const;
		T* operator->() const;
		T* Lock() const;

		template<typename U>
		Ref<U> As() const;
	};

	using Item_r = Ref<Item>;
	using FDItem_r = Ref<FDItem>;

	/***********************************************************************************************************/
	// Timer
	/***********************************************************************************************************/

	struct Timer;
	using Timer_r = Ref<Timer>;
	struct Timer : Item, xx::TimeoutBase {
		// 返回 ep
		virtual TimeoutManager* GetTimeoutManager() override;

		// 位于 ep->timers 的下标
		int indexAtContainer = -1;

		// 时间到达时触发. 如果想实现 repeat 效果, 就在函数返回前 自己 timer->SetTimeout
		std::function<void(Timer_r const& timer)> onFire;

		// 负责触发 onFire
		virtual void OnTimeout() override;
	};


	/***********************************************************************************************************/
	// TcpPeer
	/***********************************************************************************************************/

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

		virtual void OnEpollEvent(uint32_t const& e) override;

		// 数据接收事件: 从 recv 拿数据. 默认实现为 echo
		virtual void OnReceive();

		// 用于处理超时掐线
		virtual void OnTimeout() override;

		// 断线事件
		inline virtual void OnDisconnect(int const& reason) {}

		// Buf 对象塞队列并开始发送。相关信息需参考 Buf 构造函数
		int Send(xx::Buf&& data);
		int Flush();

	protected:
		int Write();
	};

	using TcpPeer_r = Ref<TcpPeer>;
	using TcpPeer_u = std::unique_ptr<TcpPeer>;


	/***********************************************************************************************************/
	// TcpListener
	/***********************************************************************************************************/

	struct TcpListener : FDItem {
		// 提供创建 peer 对象的实现
		virtual TcpPeer_u OnCreatePeer();

		// 提供为 peer 绑定事件的实现
		inline virtual void OnAccept(TcpPeer_r peer) {}

		// 调用 accept
		virtual void OnEpollEvent(uint32_t const& e) override;
	protected:
		// return fd. <0: error. 0: empty (EAGAIN / EWOULDBLOCK), > 0: fd
		int Accept(int const& listenFD);
	};


	/***********************************************************************************************************/
	// TcpConn
	/***********************************************************************************************************/

	struct Dialer;
	using Dialer_r = Ref<Dialer>;
	struct TcpConn : FDItem {
		// 指向拨号器, 方便调用其 OnConnect 函数
		Dialer_r dialer;

		// 判断是否连接成功
		virtual void OnEpollEvent(uint32_t const& e) override;
	};
	using TcpConn_r = Ref<TcpConn>;




	/***********************************************************************************************************/
	// UdpPeer
	/***********************************************************************************************************/

	struct UdpPeer : FDItem {
		// 存放该 udp socket 占用的是哪个本地端口	// todo: 0 自适应还需要去提取
		int port = -1;

		// 处理数据接收
		virtual void OnEpollEvent(uint32_t const& e) override;

		// 处理数据到达事件. 默认实现为 echo. 使用 sendto 发回收到的数据.
		virtual void OnReceive(sockaddr* fromAddr, char const* const& buf, std::size_t const& len);

		// 直接封装 sendto 函数
		int SendTo(sockaddr* toAddr, char const* const& buf, std::size_t const& len);
	};
	using UdpPeer_r = Ref<UdpPeer>;


	/***********************************************************************************************************/
	// UdpListener
	/***********************************************************************************************************/

	struct KcpPeer;
	using KcpPeer_r = Ref<KcpPeer>;
	using KcpPeer_u = std::unique_ptr<KcpPeer>;
	struct UdpListener : UdpPeer {
		// 自增生成
		uint32_t convId = 0;

		// 握手超时时间
		int handShakeTimeoutMS = 3000;

		// 判断收到的数据内容, 模拟握手， 最后产生能 KcpPeer
		virtual void OnReceive(sockaddr* fromAddr, char const* const& buf, std::size_t const& len);

		// 覆盖并提供创建 peer 对象的实现. 返回 nullptr 表示创建失败
		virtual KcpPeer_u OnCreatePeer();

		// 覆盖并提供为 peer 绑定事件的实现.
		inline virtual void OnAccept(KcpPeer_r const& peer) {}

		// 杀掉相关 kcp peers?
		//void OnDisconnect(int const& reason);
	};
	using UdpListener_r = Ref<UdpListener>;

	/***********************************************************************************************************/
	// KcpPeer
	/***********************************************************************************************************/

	struct KcpPeer : Item, xx::TimeoutBase {
		// ep 本身就是 timeout 管理器
		virtual TimeoutManager* GetTimeoutManager() override;

		// 用于收发数据的物理 udp peer
		UdpPeer_r owner;

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
		virtual void OnReceive();

		// 发数据( 数据可能滞留一会儿等待合并 )
		int Send(uint8_t const* const& buf, ssize_t const& dataLen);

		// 立刻发送, 停止等待
		virtual void Flush();

		// 被 ep 调用.
		// 受帧循环驱动. 帧率越高, kcp 工作效果越好. 典型的频率为 100 fps
		virtual void UpdateKcpLogic();

		// 被 owner 调用.
		// 塞数据到 kcp
		int Input(uint8_t* const& recvBuf, uint32_t const& recvLen);

		inline virtual void OnDisconnect(int const& reason) {};

		// 超时自杀
		virtual void OnTimeout() override;

		~KcpPeer();
	};


	/***********************************************************************************************************/
	// KcpConn
	/***********************************************************************************************************/

	struct KcpConn : UdpPeer, xx::TimeoutBase {
		// 指向拨号器, 方便调用其 OnConnect 函数
		Dialer_r dialer;
		sockaddr_in6 tarAddr;	// fill by dialer
		uint32_t serial = 0;	// fill by dialer: = ++ep->autoIncKcpSerial

		// 如果连接成功( 收到返回的握手包 )就 call dialer Finish
		virtual void OnReceive(sockaddr* fromAddr, char const* const& buf, std::size_t const& len);

		// 如果时间到了就 自动续命 并且发送 握手用数据
		virtual void OnTimeout() override;

		// ep 本身就是 timeout 管理器
		virtual TimeoutManager* GetTimeoutManager() override;
	};
	using KcpConn_r = Ref<KcpConn>;


	/***********************************************************************************************************/
	// Dialer
	/***********************************************************************************************************/

	enum class Protocol {
		Tcp,
		Kcp,
		Both
	};

	struct Dialer : Item, xx::TimeoutBase {
		// 要连的地址数组
		std::vector<sockaddr_in6> addrs;

		// 内部连接对象. 拨号完毕后会被清空
		std::vector<TcpConn_r> conns;
		std::vector<KcpConn_r> connsKcp;

		// 向 addrs 追加地址. 如果地址转换错误将返回非 0
		int AddAddress(std::string const& ip, int const& port);

		// 开始拨号。会遍历 addrs 为每个地址创建一个 ?cpConn 连接
		// 保留先连接上的 socket fd, 创建 Peer 并触发 OnConnect 事件. 
		// 如果超时，也触发 OnConnect，参数为 nullptr
		int Dial(int const& timeoutFrames, Protocol const& protocol = Protocol::Both);

		// 返回是否正在拨号
		bool Busy();

		// 停止拨号
		void Stop();

		// 存个空值备用 以方便返回引用
		TcpPeer_r emptyPeer;
		KcpPeer_r emptyPeerKcp;

		// 连接成功或超时后触发
		virtual void OnConnect(TcpPeer_r const& peer) = 0;
		virtual void OnConnectKcp(KcpPeer_r const& peer) = 0;

		// 覆盖并提供创建 peer 对象的实现. 返回 nullptr 表示创建失败
		virtual TcpPeer_u OnCreatePeer();
		virtual KcpPeer_u OnCreatePeerKcp();

		// 超时表明所有连接都没有脸上. 触发 OnConnect( nullptr )
		virtual void OnTimeout() override;

		~Dialer();

		// ep 本身就是 timeout 管理器
		virtual TimeoutManager* GetTimeoutManager() override;

	protected:
		int NewTcpConn(sockaddr_in6 const& addr);
		int NewKcpConn(sockaddr_in6 const& addr);
	};


	/***********************************************************************************************************/
	// Context
	/***********************************************************************************************************/

	struct Context : TimeoutManager {
		// fd 到 处理类 的 下标映射
		inline static std::array<FDItem*, 40000> fdMappings;

		// 所有类实例唯一容器。外界用 Ref 来存引用. 自带自增版本号管理
		ItemPool<Item_u> items;

		// kcp conv 值与 peer 的映射。KcpPeer 析构时从该字典移除 key
		xx::Dict<uint32_t, KcpPeer*> kcps;

		// 带超时的握手信息字典 key: ip:port   value: conv, nowMS
		xx::Dict<std::string, std::pair<uint32_t, int64_t>> shakes;

		// 提供自增版本号 for kcp conn
		uint32_t autoIncKcpSerial = 0;

		// epoll_wait 事件存储
		std::array<epoll_event, 4096> events;

		// 存储的 epoll fd
		int efd = -1;

		// 对于一些返回值非 int 的函数, 具体错误码将存放于此
		int lastErrorNumber = 0;

		// 共享buf for kcp read 等
		std::array<char, 65536> sharedBuf;

		// 公共只读: 每帧开始时更新一下
		int64_t nowMS = 0;

		// Run 时填充, 以便于局部获取并转换时间单位
		double frameRate = 1;



		// wheelLen: 定时器轮子尺寸( 按帧 )
		Context(std::size_t const& wheelLen = 1 << 12);

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

		template<typename T>
		T* AddItem(std::unique_ptr<T>&& item, int const& fd = -1);

		/********************************************************/
		// 下面是外部主要使用的函数

		// 将秒转为帧数
		inline int ToFrames(double const& secs) { return (int)(frameRate * secs); }


		// 帧逻辑可以覆盖这个函数. 返回非 0 将令 Run 退出. 
		inline virtual int Update() { return 0; }


		// 开始运行并尽量维持在指定帧率. 临时拖慢将补帧
		int Run(double const& frameRate = 60.3);

		// 创建 监听器	// todo: 支持填写ip, 支持传入复用 fd
		template<typename T = TcpListener, typename ...Args>
		Ref<T> CreateTcpListener(int const& port, Args&&... args);

		// 创建 连接 peer
		template<typename T = Dialer, typename ...Args>
		Ref<T> CreateDialer(Args&&... args);

		// 创建 timer
		template<typename T = Timer, typename ...Args>
		Ref<T> CreateTimer(int const& interval, std::function<void(Timer_r const& timer)>&& cb, Args&&...args);

		// 创建 UdpPeer
		template<typename T = UdpPeer, typename ...Args>
		Ref<T> CreateUdpPeer(int const& port, Args&&... args);
	};



	/***********************************************************************************************************/
	// Util funcs
	/***********************************************************************************************************/

	// ip, port 转为 addr
	int FillAddress(std::string const& ip, int const& port, sockaddr_in6& addr);
}
