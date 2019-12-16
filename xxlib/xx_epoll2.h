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
#include <readline/readline.h>
#include <readline/history.h>

#include "ikcp.h"
#include "xx_bbuffer.h"
#include "xx_dict.h"
#include "xx_queue.h"
#include "xx_buf.h"
#include "xx_buf_queue.h"
#include "xx_itempool.h"


// todo: ͳһ���������� return -?


// ע�⣺

// ���е��� �麯�� �ĵط��Ƿ��� alive ���? �����ϲ㺯���Ƿ��� call Dispose

// ��Ϊ gcc ɵ�ƣ��ᵼ������ɱ�����Ա������һ��Ҫ������Ҫ�ĳ�Ա������"ջ"�������ٵ��ú�����������쳣. ����Ҫ����ɢ�Ų�.
// ������ע��λ������ �������� ����  // ��Ҫ�����ܵ�����ʵ����ɱ  ������ɢ�Է������Լ��γɵ��� & ����Ƿ�����ɱ�Ĺ淶

// ������������� Dispose �޷����� ������ override �Ĳ���, �����
// �ϲ���ֻ�����Լ�������, ����������ʱ�޷������ϲ����Ա�� override �ĺ���
// �����������޷�ִ�� shared_from_this



namespace xx {
	// ���� sockaddr*
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
		// ָ��������
		Context* ep = nullptr;

		// linux ϵͳ�ļ�������. �ò��Ͼͱ���Ĭ��ֵ
		int fd = -1;

		// �����õ��������������ĳ�ʼ������( �������� timer, ���װ�ɶ�� )
		inline virtual void Init() {};

		// epoll �¼�����. �ò��ϲ���ʵ��
		inline virtual void OnEpollEvent(uint32_t const& e) {}

		// item ���������±�
		int indexAtContainer = -1;

		// �� ep->items �Ƴ��Լ�, ������������
		void Dispose();

		// �ر� fd( �����Ϊ -1, �ҽ��ӳ�� )
		virtual ~Item();
	};
	using Item_u = std::unique_ptr<Item>;


	/***********************************************************************************************************/
	// Ref
	/***********************************************************************************************************/

	// ��� Item �� ������αָ��. ����������ÿ�ζ������Ƿ�ʧЧ. ʧЧ���Ա� try ����
	template<typename T>
	struct Ref {
		ItemPool<Item_u>* items = nullptr;
		int index = -1;
		int64_t version = 0;

		// ��� ptr ����ȡ ep & indexAtContainer. ����Ҫ��֤��Щֵ��Ч
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

	// �����������ָ���ר����. ֱ��ӳ�䵽 STDIN_FILENO ( fd == 0 )
	struct CommandHandler : Item {
		inline static CommandHandler* self = nullptr;

		CommandHandler();
		static void ReadLineCallback(char* line);
		static char** CompleteCallback(const char* text, int start, int end);
		static char* CompleteGenerate(const char* text, int state);
		virtual void OnEpollEvent(uint32_t const& e) override;
		virtual ~CommandHandler();

	protected:
		// ���� row ���ݲ����� cmd �� handler
		void Exec(char const* const& row, size_t const& len);
	};


	/***********************************************************************************************************/
	// ItemTimeout
	/***********************************************************************************************************/

	// ��Ҫ�Դ���ʱ���ܵ� item �ɼ̳�
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
		// ʱ�䵽��ʱ����. �����ʵ�� repeat Ч��, ���ں�������ǰ �Լ� timer->SetTimeout
		std::function<void(Timer_r const& timer)> onFire;

		// ��ʱ���� onFire + ��ѡ Dispose
		virtual void OnTimeout() override;
	};


	/***********************************************************************************************************/
	// Peer
	/***********************************************************************************************************/

	struct Peer : ItemTimeout {
		// �Է��� addr( udp �յ�����ʱ�ͻ�ˢ���������. Send ������������� )
		sockaddr_in6 addr;

		// �������öѻ�����
		xx::List<char> recv;

		// ���������ڴ���������
		size_t readBufLen = 65536;


		// ���ݽ����¼�: �� recv ������. Ĭ��ʵ��Ϊ echo
		virtual void OnReceive();

		// �����¼�
		virtual void OnDisconnect(int const& reason);

		// buf + len �����в���ʼ����
		virtual int Send(char const* const& buf, size_t const& len) = 0;

		// Buf ���������в���ʼ���͡����� BBuffer �����Ż�Ⱥ���Ƚϱ���. �����Ϣ��ο� Buf ���캯��
		virtual int Send(xx::Buf&& data) = 0;

		// ���̿�ʼ��������
		virtual int Flush() = 0;

	protected:
		// ��ʱ���� Disconnect(-4) + Dispose
		virtual void OnTimeout() override;
	};

	using Peer_r = Ref<Peer>;
	using Peer_u = std::unique_ptr<Peer>;



	/***********************************************************************************************************/
	// TcpPeer
	/***********************************************************************************************************/

	struct TcpPeer : Peer {

		// �Ƿ����ڷ���( �ǣ����� sendQueue �ղ��գ������� write, ֻ���� sendQueue )
		bool writing = false;

		// �����Ͷ���
		xx::BufQueue sendQueue;

		// ÿ fd ÿһ�ο�д, д��ĳ�������( ϣ����ʵ�ֵ����������·�ʱ���� socket ��ƽ��ռ�ô��� )
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
		// �ṩ���� peer �����ʵ��
		virtual TcpPeer_u OnCreatePeer();

		// �ṩΪ peer ���¼���ʵ��
		inline virtual void OnAccept(TcpPeer_r const& peer) {}

		// ���� accept
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
		// ָ�򲦺���, ��������� OnConnect ����
		Dialer_r dialer;

		// �ж��Ƿ����ӳɹ�
		virtual void OnEpollEvent(uint32_t const& e) override;
	};
	using TcpConn_r = Ref<TcpConn>;



	/***********************************************************************************************************/
	// UdpPeer
	/***********************************************************************************************************/

	struct UdpPeer : Peer {
		virtual void OnEpollEvent(uint32_t const& e) override;
		virtual int Send(xx::Buf&& data) override;
		virtual int Send(char const* const& buf, size_t const& len) override;	// ֱ�ӷ���, ��ѹ����
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

		// ��������
		uint32_t convId = 0;

		// kcp conv ֵ�� peer ��ӳ�䡣KcpPeer ����ʱ�Ӹ��ֵ��Ƴ� key
		xx::Dict<uint32_t, KcpPeer*> kcps;

		// ����ʱ��������Ϣ�ֵ� key: ip:port   value: conv, nowMS
		xx::Dict<std::string, std::pair<uint32_t, int64_t>> shakes;

		// ����ÿ֡�ص� SetTimeout(1)
		virtual void Init() override;

		// ���Ӵ���֮����ᴥ��
		virtual KcpPeer_u OnCreatePeer();

		// ���Ӵ����ɹ���ᴥ��
		inline virtual void OnAccept(KcpPeer_r const& peer) {}

		// ÿ֡ call kcps UpdateKcpLogic, ����ʱ��������
		virtual void OnTimeout() override;

		// ��� kcps
		virtual ~KcpListener();
	protected:
		// �ж��յ�����������, ģ�����֣� �������� KcpPeer
		virtual void OnReceive() override;
	};
	using KcpListener_r = Ref<KcpListener>;



	/***********************************************************************************************************/
	// KcpPeer
	/***********************************************************************************************************/

	struct KcpPeer : Peer {
		// �����շ����ݵ����� udp peer
		UdpPeer_r owner;

		// for ���ٽ���ɾ��
		int indexAtKcps = -1;

		// kcp ���������
		ikcpcb* kcp = nullptr;
		uint32_t conv = 0;
		int64_t createMS = 0;
		uint32_t nextUpdateMS = 0;

		// �ڲ�����������������ȷ�� conv �����
		int InitKcp();

		// �� ep ����. ��֡ѭ������. ֡��Խ��, kcp ����Ч��Խ��. ���͵�Ƶ��Ϊ 100 fps
		virtual void UpdateKcpLogic();

		// �� owner ����. �����ݵ� kcp
		void Input(char* const& recvBuf, size_t const& recvLen);

		// ���� kcp ����, �� ep->kcps �Ƴ�
		~KcpPeer();

		virtual int Send(xx::Buf&& data) override;
		virtual int Send(char const* const& buf, size_t const& len) override;
		virtual int Flush() override;
	};


	/***********************************************************************************************************/
	// KcpConn
	/***********************************************************************************************************/

	// ���ųɹ������Ϊ���� KcpPeer �ϵ�����peer
	struct KcpConn : UdpPeer {
		// ָ�򲦺���, ��������� OnConnect ����
		Dialer_r dialer;

		// ָ�� kcp peer, ��������� Input
		KcpPeer* peer = nullptr;

		// ��������. ���Ϊ�汾��. ÿ�ε����������Ӧ���������. init: = ++ep->autoIncKcpSerial
		uint32_t serial = 0;

		// һ��ʼ�ͷ��������� timer
		virtual void Init() override;

		// ���ʱ�䵽�˾� �Զ����� ���ҷ��� ����������
		virtual void OnTimeout() override;

		// ������ӳɹ�( �յ����ص����ְ� )�� call dialer Finish
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
		// Ҫ���ĵ�ַ����. ��Э����
		std::vector<std::pair<sockaddr_in6, Protocol>> addrs;

		// �� addrs ׷�ӵ�ַ. �����ַת�����󽫷��ط� 0
		int AddAddress(std::string const& ip, int const& port, Protocol const& protocol = Protocol::Both);

		// ��ʼ���š������ addrs Ϊÿ����ַ����һ�� ?cpConn ����
		// �����������ϵ� socket fd, ���� Peer ������ OnConnect �¼�. 
		// �����ʱ��Ҳ���� OnConnect������Ϊ nullptr
		int Dial(int const& timeoutFrames);

		// �����Ƿ����ڲ���
		bool Busy();

		// ֹͣ���� ������ conns. ���� addrs.
		void Stop();

		// ���ӳɹ���ʱ�󴥷�
		virtual void OnConnect(Peer_r const& peer) = 0;

		// ���ǲ��ṩ���� peer �����ʵ��. ���� nullptr ��ʾ����ʧ��
		virtual Peer_u OnCreatePeer(Protocol const& protocol);

		// Stop()
		~Dialer();

		// �����ֵ���� �Է��㷵������
		Peer_r emptyPeer;

		// �ڲ����Ӷ���. ������Ϻ�ᱻ���
		std::vector<Item_r> conns;

		// ��ʱ�����������Ӷ�û������. ���� OnConnect( nullptr )
		virtual void OnTimeout() override;
	protected:
		// ������Э�鴴�� Conn ����
		int NewTcpConn(sockaddr_in6 const& addr);
		int NewKcpConn(sockaddr_in6 const& addr);
	};


	/***********************************************************************************************************/
	// Context
	/***********************************************************************************************************/

	struct Context {
		// ������ʵ��Ψһ����������� Ref ��������. �Դ������汾�Ź���
		ItemPool<Item_u> items;

		// fd �� ������* �� ӳ��
		std::array<Item*, 40000> fdMappings;



		// ͨ�� Dialer ������, owner ָ�� KcpConn �� client kcp peers
		std::vector<KcpPeer*> kcps;

		// �ṩ�����汾�� for kcp conn
		uint32_t autoIncKcpSerial = 0;



		// epoll_wait �¼��洢
		std::array<epoll_event, 4096> events;

		// �洢�� epoll fd
		int efd = -1;



		// ʱ����. ֻ��ָ������, �������ڴ�
		std::vector<ItemTimeout*> wheel;

		// ָ��ʱ���ֵ��α�
		int cursor = 0;


		/********************************************************/
		// �����⼸���û����Զ�


		// ����һЩ����ֵ�� int �ĺ���, ��������뽫����ڴ�
		int lastErrorNumber = 0;

		// ����ֻ��: ÿ֡��ʼʱ����һ��
		int64_t nowMS = 0;

		// Run ʱ���, �Ա��ھֲ���ȡ��ת��ʱ�䵥λ
		double frameRate = 1;


		/********************************************************/
		// �����⼸���û����Զ�д

		// ִ�б�־λ�����Ҫ�˳����޸���
		bool running = true;

		// for SendRequest( .... , 0 )
		int64_t defaultRequestTimeoutMS = 15000;

		// for recv safe check
		uint32_t maxPackageLength = 1024 * 256;

		// ���÷����л� bb. ֱ���� Reset ���滻�ڴ�ʹ��. 
		BBuffer recvBB;

		// �������л� bb
		BBuffer sendBB;

		// �������л� bb( ����ָ��� )
		BBuffer_s sharedBB = xx::Make<BBuffer>();

		// ���� buf( ������ STDIN ������� )
		std::array<char, 65536> buf;

		// ���� args( ������ cmds ���� )
		std::vector<std::string> args;

		// ӳ��ͨ�� stdin ������ָ��Ĵ�����. ȥ�ո� ȥ tab ���һ��������Ϊ key. ʣ�ಿ����Ϊ args
		std::unordered_map<std::string, std::function<void(std::vector<std::string> const& args)>> cmds;

		//
		/********************************************************/



		// ָ�� �Ƿ�Ϊ���߳�����( ���̵߳�Ҫ������������� )���Լ�ʱ���ֳ���( Ҫ��Ϊ 2^n )
		Context(bool isMainThread = true, size_t const& wheelLen = 1 << 12);

		virtual ~Context();

		Context(Context const&) = delete;
		Context& operator=(Context const&) = delete;


		// ���������� socket fd ������. < 0: error
		int MakeSocketFD(int const& port, int const& sockType = SOCK_STREAM); // SOCK_DGRAM

		// ��� fd �� epoll ����. return !0: error
		int Ctl(int const& fd, uint32_t const& flags, int const& op = EPOLL_CTL_ADD);

		// �رղ��� epoll �Ƴ�����
		int CloseDel(int const& fd);

		// ����һ�� epoll wait. �ɴ��볬ʱʱ��. 
		int Wait(int const& timeoutMS);


		// ÿ֡����һ�� ������ timer
		inline void UpdateTimeoutWheel();

		// ���� kcp �� Update
		void UpdateKcps();

		template<typename T>
		T* AddItem(std::unique_ptr<T>&& item, int const& fd = -1);

		/********************************************************/
		// �������ⲿ��Ҫʹ�õĺ���

		// ����תΪ֡��
		inline int ToFrames(double const& secs) { return (int)(frameRate * secs); }


		// ֡�߼����Ը����������. ���ط� 0 ���� Run �˳�. 
		inline virtual int Update() { return 0; }


		// ��ʼ���в�����ά����ָ��֡��. ��ʱ��������֡
		int Run(double const& frameRate = 60.3);

		// ���� TCP ������
		template<typename T = TcpListener, typename ...Args>
		Ref<T> CreateTcpListener(int const& port, Args&&... args);

		// ���� ���� peer
		template<typename T = Dialer, typename ...Args>
		Ref<T> CreateDialer(Args&&... args);

		// ���� timer
		template<typename T = Timer, typename ...Args>
		Ref<T> CreateTimer(int const& interval, std::function<void(Timer_r const& timer)>&& cb, Args&&...args);

		// ���� UdpPeer �� KcpListener
		template<typename T = UdpPeer, typename ...Args>
		Ref<T> CreateUdpPeer(int const& port, Args&&... args);


		// ���� TCP ������, ���븴�� fd
		template<typename T = TcpListener, typename ...Args>
		Ref<T> CreateSharedTcpListener(int const& fd, Args&&... args);
	};



	/***********************************************************************************************************/
	// Util funcs
	/***********************************************************************************************************/

	// ip, port תΪ addr
	int FillAddress(std::string const& ip, int const& port, sockaddr_in6& addr);
}
