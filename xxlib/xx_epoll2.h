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
#include <termios.h>

#include "ikcp.h"
#include "xx_bbuffer.h"
#include "xx_dict.h"
#include "xx_queue.h"
#include "xx_buf.h"
#include "xx_buf_queue.h"
#include "xx_itempool.h"


// todo: 统一递增填充各种 return -?


// 注意：

// 所有调用 虚函数 的地方是否有 alive 检测? 需检测上层函数是否已 call Dispose

// 因为 gcc 傻逼，会导致类自杀的类成员函数，一定要复制需要的成员参数到"栈"变量，再调用函数，避免出异常. 且需要逐级扩散排查.
// 考虑在注释位置增加 文字描述 类似  // 重要：可能导致类实例自杀  并逐级扩散以方便检查以及形成调用 & 检测是否已自杀的规范

// 基类析构发起的 Dispose 无法调用 派生类 override 的部分, 需谨慎
// 上层类只析构自己的数据, 到基类析构时无法访问上层类成员与 override 的函数
// 析构过程中无法执行 shared_from_this



namespace xx {
	// 适配 sockaddr*
	template<typename T>
	struct SFuncs<T, std::enable_if_t<std::is_same_v<sockaddr*, std::decay_t<T>>>> {
		static inline void WriteTo(std::string& s, T const& in) noexcept {
			char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
			if (!getnameinfo(in, in->sa_family == AF_INET6 ? INET6_ADDRSTRLEN : INET_ADDRSTRLEN
				, hbuf, sizeof hbuf, sbuf, sizeof sbuf, NI_NUMERICHOST | NI_NUMERICSERV)) {
				xx::Append(s, (char*)hbuf, ":", (char*)sbuf);
			}
		}
	};
	template<typename T>
	struct SFuncs<T, std::enable_if_t<std::is_same_v<sockaddr_in, std::decay_t<T>> || std::is_same_v<sockaddr_in6, std::decay_t<T>>>> {
		static inline void WriteTo(std::string& s, T const& in) noexcept {
			return SFuncs<sockaddr*, void>::WriteTo(s, (sockaddr*)&in);
		}
	};
}

namespace xx::Epoll {

	/***********************************************************************************************************/
	// Item
	/***********************************************************************************************************/

	struct Context;
	struct Item {
		// 指向总容器
		Context* ep = nullptr;

		// linux 系统文件描述符. 用不上就保持默认值
		int fd = -1;

		// 留个拿到依赖填充完整后的初始化口子( 比如启动 timer, 发首包啥的 )
		inline virtual void Init() {};

		// epoll 事件处理. 用不上不必实现
		inline virtual void OnEpollEvent(uint32_t const& e) {}

		// item 所在容器下标
		int indexAtContainer = -1;

		// 从 ep->items 移除自己, 进而触发析构
		void Dispose();

		// 关闭 fd( 如果不为 -1, 且解除映射 )
		virtual ~Item();
	};
	using Item_u = std::unique_ptr<Item>;


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
		Ref(Ref&&) = default;
		Ref& operator=(Ref const&) = default;
		Ref& operator=(Ref&&) = default;

		template<typename U>
		Ref& operator=(std::enable_if_t<std::is_base_of_v<T, U>, Ref<U>> const& o);
		template<typename U>
		Ref& operator=(std::enable_if_t<std::is_base_of_v<T, U>, Ref<U>>&& o);

		template<typename U>
		Ref<U> As() const;

		operator bool() const;
		T* operator->() const;
		T* Lock() const;
		template<typename U = T>
		void Reset(U* const& ptr = nullptr);
	};

	template<typename A, typename B>
	inline bool operator==(Ref<A> const& a, Ref<B> const& b) {
		return a.Lock() == b.Lock();
	}

	template<typename A, typename B>
	inline bool operator!=(Ref<A> const& a, Ref<B> const& b) {
		return a.Lock() != b.Lock();
	}

	using Item_r = Ref<Item>;


	/***********************************************************************************************************/
	// CommandHandler
	/***********************************************************************************************************/

	// 处理键盘输入指令的专用类. 直接映射到 STDIN_FILENO ( fd == 0 )
	struct CommandHandler : Item {
		std::string row;
		size_t cursor = 0;
		virtual void OnEpollEvent(uint32_t const& e) override;
		virtual ~CommandHandler();

	protected:
		// 解析 row 内容并调用 cmd 绑定 handler
		void Exec();

		// 同步显示光标右侧内容. print 右侧字串+' ' 并退格 sizeof(右侧字串)+1
		void PrintCursorToEnd();

		// 各种直接移动光标
		void Right();
		void Left();
		void Backspace();
		void Home();
		void End();
		void Del();
		void PageUp();
		void PageDown();
		void Up();
		void Down();

		// 正常字符插入或追加
		void Insert(char const& c);
		void Append(char const& c);
	};


	/***********************************************************************************************************/
	// ItemTimeout
	/***********************************************************************************************************/

	// 需要自带超时功能的 item 可继承
	struct ItemTimeout : Item {
		int timeoutIndex = -1;
		ItemTimeout* timeoutPrev = nullptr;
		ItemTimeout* timeoutNext = nullptr;
		int SetTimeout(int const& interval);
		virtual void OnTimeout() = 0;
		~ItemTimeout();
	};


	/***********************************************************************************************************/
	// Timer
	/***********************************************************************************************************/

	struct Timer;
	using Timer_r = Ref<Timer>;
	struct Timer : ItemTimeout {
		// 时间到达时触发. 如果想实现 repeat 效果, 就在函数返回前 自己 timer->SetTimeout
		std::function<void(Timer_r const& timer)> onFire;

		// 超时触发 onFire + 可选 Dispose
		virtual void OnTimeout() override;
	};


	/***********************************************************************************************************/
	// Peer
	/***********************************************************************************************************/

	struct Peer : ItemTimeout {
		// 对方的 addr( udp 收到数据时就会刷新这个属性. Send 将采用这个属性 )
		sockaddr_in6 addr;

		// 收数据用堆积容器
		xx::List<char> recv;

		// 读缓冲区内存扩容增量
		size_t readBufLen = 65536;


		// 数据接收事件: 从 recv 拿数据. 默认实现为 echo
		virtual void OnReceive();

		// 断线事件
		virtual void OnDisconnect(int const& reason);

		// buf + len 塞队列并开始发送
		virtual int Send(char const* const& buf, size_t const& len) = 0;

		// Buf 对象塞队列并开始发送。传递 BBuffer 或者优化群发比较便利. 相关信息需参考 Buf 构造函数
		virtual int Send(xx::Buf&& data) = 0;

		// 立刻开始发送数据
		virtual int Flush() = 0;

	protected:
		// 超时触发 Disconnect(-4) + Dispose
		virtual void OnTimeout() override;
	};

	using Peer_r = Ref<Peer>;
	using Peer_u = std::unique_ptr<Peer>;



	/***********************************************************************************************************/
	// TcpPeer
	/***********************************************************************************************************/

	struct TcpPeer : Peer {

		// 是否正在发送( 是：不管 sendQueue 空不空，都不能 write, 只能塞 sendQueue )
		bool writing = false;

		// 待发送队列
		xx::BufQueue sendQueue;

		// 每 fd 每一次可写, 写入的长度限制( 希望能实现当大量数据下发时各个 socket 公平的占用带宽 )
		size_t sendLenPerFrame = 65536;

		virtual void OnEpollEvent(uint32_t const& e) override;
		virtual int Send(xx::Buf&& data) override;
		virtual int Send(char const* const& buf, size_t const& len) override;
		virtual int Flush() override;
	protected:
		int Write();
	};

	using TcpPeer_r = Ref<TcpPeer>;
	using TcpPeer_u = std::unique_ptr<TcpPeer>;



	/***********************************************************************************************************/
	// TcpListener
	/***********************************************************************************************************/

	struct TcpListener : Item {
		typedef TcpPeer PeerType;
		// 提供创建 peer 对象的实现
		virtual TcpPeer_u OnCreatePeer();

		// 提供为 peer 绑定事件的实现
		inline virtual void OnAccept(TcpPeer_r const& peer) {}

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
	struct TcpConn : Item {
		// 指向拨号器, 方便调用其 OnConnect 函数
		Dialer_r dialer;

		// 判断是否连接成功
		virtual void OnEpollEvent(uint32_t const& e) override;
	};
	using TcpConn_r = Ref<TcpConn>;



	/***********************************************************************************************************/
	// UdpPeer
	/***********************************************************************************************************/

	struct UdpPeer : Peer {
		virtual void OnEpollEvent(uint32_t const& e) override;
		virtual int Send(xx::Buf&& data) override;
		virtual int Send(char const* const& buf, size_t const& len) override;	// 直接发送, 不压队列
		virtual int Flush() override;
	};
	using UdpPeer_r = Ref<UdpPeer>;



	/***********************************************************************************************************/
	// KcpListener
	/***********************************************************************************************************/

	struct KcpPeer;
	using KcpPeer_r = Ref<KcpPeer>;
	using KcpPeer_u = std::unique_ptr<KcpPeer>;
	struct KcpListener : UdpPeer {
		typedef KcpPeer PeerType;

		// 自增生成
		uint32_t convId = 0;

		// kcp conv 值与 peer 的映射。KcpPeer 析构时从该字典移除 key
		xx::Dict<uint32_t, KcpPeer*> kcps;

		// 带超时的握手信息字典 key: ip:port   value: conv, nowMS
		xx::Dict<std::string, std::pair<uint32_t, int64_t>> shakes;

		// 启动每帧回调 SetTimeout(1)
		virtual void Init() override;

		// 连接创建之初后会触发
		virtual KcpPeer_u OnCreatePeer();

		// 连接创建成功后会触发
		inline virtual void OnAccept(KcpPeer_r const& peer) {}

		// 每帧 call kcps UpdateKcpLogic, 清理超时握手数据
		virtual void OnTimeout() override;

		// 清除 kcps
		virtual ~KcpListener();
	protected:
		// 判断收到的数据内容, 模拟握手， 最后产生能 KcpPeer
		virtual void OnReceive() override;
	};
	using KcpListener_r = Ref<KcpListener>;



	/***********************************************************************************************************/
	// KcpPeer
	/***********************************************************************************************************/

	struct KcpPeer : Peer {
		// 用于收发数据的物理 udp peer
		UdpPeer_r owner;

		// for 快速交焕删除
		int indexAtKcps = -1;

		// kcp 相关上下文
		ikcpcb* kcp = nullptr;
		uint32_t conv = 0;
		int64_t createMS = 0;
		uint32_t nextUpdateMS = 0;

		// 内部函数：创建过程中确定 conv 后调用
		int InitKcp();

		// 被 ep 调用. 受帧循环驱动. 帧率越高, kcp 工作效果越好. 典型的频率为 100 fps
		virtual void UpdateKcpLogic();

		// 被 owner 调用. 塞数据到 kcp
		void Input(char* const& recvBuf, size_t const& recvLen);

		// 回收 kcp 对象, 从 ep->kcps 移除
		~KcpPeer();

		virtual int Send(xx::Buf&& data) override;
		virtual int Send(char const* const& buf, size_t const& len) override;
		virtual int Flush() override;
	};


	/***********************************************************************************************************/
	// KcpConn
	/***********************************************************************************************************/

	// 拨号成功后变身为挂在 KcpPeer 上当物理peer
	struct KcpConn : UdpPeer {
		// 指向拨号器, 方便调用其 OnConnect 函数
		Dialer_r dialer;

		// 指向 kcp peer, 方便调用其 Input
		KcpPeer* peer = nullptr;

		// 握手数据. 理解为版本号. 每次递增避免晚回应包制造干扰. init: = ++ep->autoIncKcpSerial
		uint32_t serial = 0;

		// 一开始就发包并启动 timer
		virtual void Init() override;

		// 如果时间到了就 自动续命 并且发送 握手用数据
		virtual void OnTimeout() override;

		// 如果连接成功( 收到返回的握手包 )就 call dialer Finish
		virtual void OnReceive() override;
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

	struct Dialer : ItemTimeout {
		// 要连的地址数组. 带协议标记
		std::vector<std::pair<sockaddr_in6, Protocol>> addrs;

		// 向 addrs 追加地址. 如果地址转换错误将返回非 0
		int AddAddress(std::string const& ip, int const& port, Protocol const& protocol = Protocol::Both);

		// 开始拨号。会遍历 addrs 为每个地址创建一个 ?cpConn 连接
		// 保留先连接上的 socket fd, 创建 Peer 并触发 OnConnect 事件. 
		// 如果超时，也触发 OnConnect，参数为 nullptr
		int Dial(int const& timeoutFrames);

		// 返回是否正在拨号
		bool Busy();

		// 停止拨号 并清理 conns. 保留 addrs.
		void Stop();

		// 连接成功或超时后触发
		virtual void OnConnect(Peer_r const& peer) = 0;

		// 覆盖并提供创建 peer 对象的实现. 返回 nullptr 表示创建失败
		virtual Peer_u OnCreatePeer(Protocol const& protocol);

		// Stop()
		~Dialer();

		// 存个空值备用 以方便返回引用
		Peer_r emptyPeer;

		// 内部连接对象. 拨号完毕后会被清空
		std::vector<Item_r> conns;

		// 超时表明所有连接都没有连上. 触发 OnConnect( nullptr )
		virtual void OnTimeout() override;
	protected:
		// 按具体协议创建 Conn 对象
		int NewTcpConn(sockaddr_in6 const& addr);
		int NewKcpConn(sockaddr_in6 const& addr);
	};


	/***********************************************************************************************************/
	// Context
	/***********************************************************************************************************/

	struct Context {
		// 所有类实例唯一容器。外界用 Ref 来存引用. 自带自增版本号管理
		ItemPool<Item_u> items;

		// fd 到 处理类* 的 映射
		std::array<Item*, 40000> fdMappings;



		// 通过 Dialer 产生的, owner 指向 KcpConn 的 client kcp peers
		std::vector<KcpPeer*> kcps;

		// 提供自增版本号 for kcp conn
		uint32_t autoIncKcpSerial = 0;



		// epoll_wait 事件存储
		std::array<epoll_event, 4096> events;

		// 存储的 epoll fd
		int efd = -1;



		// 时间轮. 只存指针引用, 不管理内存
		std::vector<ItemTimeout*> wheel;

		// 指向时间轮的游标
		int cursor = 0;


		/********************************************************/
		// 下面这几个用户可以读


		// 对于一些返回值非 int 的函数, 具体错误码将存放于此
		int lastErrorNumber = 0;

		// 公共只读: 每帧开始时更新一下
		int64_t nowMS = 0;

		// Run 时填充, 以便于局部获取并转换时间单位
		double frameRate = 1;


		/********************************************************/
		// 下面这几个用户可以读写

		// 执行标志位。如果要退出，修改它
		bool running = true;

		// for SendRequest( .... , 0 )
		int64_t defaultRequestTimeoutMS = 15000;

		// for recv safe check
		uint32_t maxPackageLength = 1024 * 256;

		// 公用反序列化 bb. 直接用 Reset 来替换内存使用. 
		BBuffer recvBB;

		// 公用序列化 bb
		BBuffer sendBB;

		// 公用序列化 bb( 智能指针版 )
		BBuffer_s sharedBB = xx::Make<BBuffer>();

		// 公用 buf( 已用于 STDIN 输入接收 )
		std::array<char, 65536> buf;

		// 公用 args( 已用于 cmdHandlers 传参 )
		std::vector<std::string> args;

		// 映射通过 stdin 进来的指令的处理函数. 去空格 去 tab 后第一个单词作为 key. 剩余部分作为 args
		std::unordered_map<std::string, std::function<void(std::vector<std::string> const& args)>> cmdHandlers;

		//
		/********************************************************/



		// 指定时间轮长度( 要求为 2^n )
		Context(size_t const& wheelLen = 1 << 12);

		virtual ~Context();


		// 创建非阻塞 socket fd 并返回. < 0: error
		int MakeSocketFD(int const& port, int const& sockType = SOCK_STREAM); // SOCK_DGRAM

		// 添加 fd 到 epoll 监视. return !0: error
		int Ctl(int const& fd, uint32_t const& flags, int const& op = EPOLL_CTL_ADD);

		// 关闭并从 epoll 移除监视
		int CloseDel(int const& fd);

		// 进入一次 epoll wait. 可传入超时时间. 
		int Wait(int const& timeoutMS);


		// 每帧调用一次 以驱动 timer
		inline void UpdateTimeoutWheel();

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

		// 创建指令处理器( 注意：因为是针对 STDIN fd == 0, 只能创建一份 )
		int CreateCommandHandler(bool const& advanceMode = false);

		// 创建 TCP 监听器
		template<typename T = TcpListener, typename ...Args>
		Ref<T> CreateTcpListener(int const& port, Args&&... args);

		// 创建 连接 peer
		template<typename T = Dialer, typename ...Args>
		Ref<T> CreateDialer(Args&&... args);

		// 创建 timer
		template<typename T = Timer, typename ...Args>
		Ref<T> CreateTimer(int const& interval, std::function<void(Timer_r const& timer)>&& cb, Args&&...args);

		// 创建 UdpPeer 或 KcpListener
		template<typename T = UdpPeer, typename ...Args>
		Ref<T> CreateUdpPeer(int const& port, Args&&... args);


		// 创建 TCP 监听器, 传入复用 fd
		template<typename T = TcpListener, typename ...Args>
		Ref<T> CreateSharedTcpListener(int const& fd, Args&&... args);
	};



	/***********************************************************************************************************/
	// Util funcs
	/***********************************************************************************************************/

	// ip, port 转为 addr
	int FillAddress(std::string const& ip, int const& port, sockaddr_in6& addr);
}
