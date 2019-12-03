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


// todo: ���к�����һ�� disposed ���?

// ע�⣺
// �����������޷�ִ�� shared_from_this
// ������������� Dispose �޷����� ������ override �Ĳ���, �����
// �ϲ���ֻ�����Լ�������, ����������ʱ�޷������ϲ����Ա�� override �ĺ���


/*
	���Ŀ�꣺��������ʹ�ø��ӳ̶�, ���ͳ��������
	���˼·���� fd �ҹ����࣬�ر��� peer, ��Ҫ�����ð�ȫʹ��. �����������������ֱ���ü��ɡ����ò�������ָ�롣
	xx::Epoll::Context ( ��� ep, ��ͬ ), ͨ��һ����Ŀֻ��Ҫȷ������������������ɺܷ���ķ���һЩ����
	
	��ָ������ṹ�� owner*, index, version
	owner*: ָ��������. ���� ep.  index: λ�����������±�.   version: �汾�ţ������ж� index �Ƿ���ʧЧ

	�˴����� epoll fd �ṹ�����ԣ���ʹ��һ��ȫ�־�̬������, ����ָ������ṹ�ɼ򻯵� owner*

*/


namespace xx::Epoll {

	/***********************************************************************************************************/
	// Item
	/***********************************************************************************************************/

	struct Context;

	struct Item {
		// ָ��������
		Context* ep = nullptr;

		// item ���������±�
		int indexAtContainer = -1;

		// �� ���� �Ƴ��Լ�, ������������
		virtual void Dispose();

		// ÿ�����������������Ҫд this->Dispose(-1);
		virtual ~Item() {}
	};

	/***********************************************************************************************************/
	// Item_r
	/***********************************************************************************************************/

	// ��� Item �� ������αָ��. ����������ÿ�ζ������Ƿ�ʧЧ. ʧЧ���Ա� try ����
	template<typename T>
	struct Item_r {
		ItemPool<std::unique_ptr<Item>>* items = nullptr;
		int index = -1;
		int64_t version = 0;

		// ��� ptr ����ȡ ep & indexAtContainer. ����Ҫ��֤��Щֵ��Ч
		Item_r(T* const& ptr);
		Item_r(std::unique_ptr<T> const& ptr);

		Item_r() = default;
		Item_r(Item_r const&) = default;
		Item_r& operator=(Item_r const&) = default;

		operator bool() const;
		T* operator->() const;
		T* Lock() const;

		template<typename U>
		Item_r<U> As() const;
	};


	/***********************************************************************************************************/
	// FDItem
	/***********************************************************************************************************/

	struct FDItem : Item {
		// linux ϵͳ�ļ�������. ͬʱҲ�� ep->fdHandlers ���±�
		//int fd = -1;	// ֱ���� indexAtContainer

		// epoll fd �¼�����. �������Լ�ֱ�� Dispose ��ɱ
		virtual void OnEpollEvent(uint32_t const& e) = 0;

		// �� ep->fdHandlers �Ƴ��Լ�, ������������
		virtual void Dispose() override;

		// �ر� fd ɶ��
		virtual ~FDItem();
	};


	/***********************************************************************************************************/
	// Timer
	/***********************************************************************************************************/

	struct Timer : Item, xx::TimeoutBase {
		// ���� ep
		virtual TimeoutManager* GetTimeoutManager() override;

		// λ�� ep->timers ���±�
		int indexAtContainer = -1;

		// ʱ�䵽��ʱ����. �����ʵ�� repeat Ч��, ���ں�������ǰ �Լ� timer->SetTimeout
		std::function<void(Timer* const& timer)> onFire;

		// ���𴥷� onFire
		virtual void OnTimeout() override;

		virtual ~Timer();
	};


	/***********************************************************************************************************/
	// TcpPeer
	/***********************************************************************************************************/

	struct TcpListener;
	struct TcpPeer : FDItem, xx::TimeoutBase {
		// ep ������� timeout ������
		virtual TimeoutManager* GetTimeoutManager() override;

		// ip
		std::string ip;

		// �������öѻ�����
		xx::List<uint8_t> recv;

		// �Ƿ����ڷ���( �ǣ����� sendQueue �ղ��գ������� write, ֻ���� sendQueue )
		bool writing = false;

		// �����Ͷ���
		xx::BufQueue sendQueue;

		// ÿ fd ÿһ�ο�д, д��ĳ�������( ϣ����ʵ�ֵ����������·�ʱ���� socket ��ƽ��ռ�ô��� )
		std::size_t sendLenPerFrame = 65536;

		// ���������ڴ���������
		std::size_t readBufLen = 65536;

		virtual void OnEpollEvent(uint32_t const& e) override;

		// ���ݽ����¼�: �� recv ������. Ĭ��ʵ��Ϊ echo
		virtual void OnReceive();

		// ���ڴ���ʱ����
		virtual void OnTimeout() override;

		// �����¼�
		inline virtual void OnDisconnect(int const& reason) {}

		~TcpPeer();

		// Buf ���������в���ʼ���͡������Ϣ��ο� Buf ���캯��
		int Send(xx::Buf&& data);
		int Flush();

	protected:
		int Write();
	};


	/***********************************************************************************************************/
	// TcpListener
	/***********************************************************************************************************/

	struct TcpListener : FDItem {
		// �ṩ���� peer �����ʵ��
		virtual std::unique_ptr<TcpPeer> OnCreatePeer();

		// �ṩΪ peer ���¼���ʵ��
		inline virtual void OnAccept(Item_r<TcpPeer> peer) {}

		// ���� accept
		virtual void OnEpollEvent(uint32_t const& e) override;
	protected:
		// return fd. <0: error. 0: empty (EAGAIN / EWOULDBLOCK), > 0: fd
		int Accept(int const& listenFD);
	};


	/***********************************************************************************************************/
	// TcpConn
	/***********************************************************************************************************/

	struct TcpDialer;
	struct TcpConn : FDItem {
		// ָ�򲦺���, ��������� OnConnect ����
		Item_r<TcpDialer> dialer;

		// �ж��Ƿ����ӳɹ�
		virtual void OnEpollEvent(uint32_t const& e) override;
	};


	/***********************************************************************************************************/
	// TcpDialer
	/***********************************************************************************************************/

	struct TcpDialer : Item, xx::TimeoutBase {
		// ep ������� timeout ������
		virtual TimeoutManager* GetTimeoutManager() override;

		// λ�� ep->items �������±�
		int indexOfContainer = -1;

		// Ҫ���ĵ�ַ����
		std::vector<sockaddr_in6> addrs;

		// �ڲ����Ӷ���. ������Ϻ�ᱻ���
		std::vector<Item_r<TcpConn>> conns;

		// �� addrs ׷�ӵ�ַ. �����ַת�����󽫷��ط� 0
		int AddAddress(std::string const& ip, int const& port);

		// ��ʼ���š������ addrs Ϊÿ����ַ����һ�� TcpConn ����
		// �����������ϵ� socket fd, ���� Peer ������ OnConnect �¼�. 
		// �����ʱ��Ҳ���� OnConnect������Ϊ nullptr
		int Dial(int const& timeoutFrames);

		// �����Ƿ����ڲ���
		bool Busy();

		// ֹͣ����
		void Stop();

		// �����ֵ���� �Է��㷵������
		Item_r<TcpPeer> emptyPeer;

		// ���ӳɹ���ʱ�󴥷�
		virtual void OnConnect(Item_r<TcpPeer> const& peer) = 0;

		// ���ǲ��ṩ���� peer �����ʵ��. ���� nullptr ��ʾ����ʧ��
		virtual std::unique_ptr<TcpPeer> OnCreatePeer();

		// ���� TcpConn ֪ͨ�Լ����ӳɹ������� fd �Ա���
		void Finish(int fd);

		// ��ʱ�����������Ӷ�û������. ���� OnConnect( nullptr )
		virtual void OnTimeout() override;

		~TcpDialer();
	};




	/***********************************************************************************************************/
	// UdpPeer
	/***********************************************************************************************************/

	struct UdpPeer : FDItem {
		// ��Ÿ� udp socket ռ�õ����ĸ����ض˿�	// todo: 0 ����Ӧ����Ҫȥ��ȡ
		int port = -1;

		// �������ݽ���
		virtual void OnEpollEvent(uint32_t const& e) override;

		// �������ݵ����¼�. Ĭ��ʵ��Ϊ echo. ʹ�� sendto �����յ�������.
		virtual void OnReceive(sockaddr* fromAddr, char const* const& buf, std::size_t const& len);

		// ֱ�ӷ�װ sendto ����
		int SendTo(sockaddr* toAddr, char const* const& buf, std::size_t const& len);
	};


	/***********************************************************************************************************/
	// UdpListener
	/***********************************************************************************************************/

	struct KcpPeer;
	struct UdpListener : UdpPeer {
		// ��������
		uint32_t convId = 0;

		// ���ֳ�ʱʱ��
		int handShakeTimeoutMS = 3000;

		// �ж��յ�����������, ģ�����֣� �������� KcpPeer
		virtual void OnReceive(sockaddr* fromAddr, char const* const& buf, std::size_t const& len);

		// ���ǲ��ṩ���� peer �����ʵ��. ���� nullptr ��ʾ����ʧ��
		virtual std::unique_ptr<KcpPeer> OnCreatePeer();

		// ���ǲ��ṩΪ peer ���¼���ʵ��. ���ط� 0 ��ʾ��ֹ accept
		inline virtual void OnAccept(Item_r<KcpPeer> const& peer) {}

		// ɱ����� kcp peers?
		//void OnDisconnect(int const& reason);
	};


	/***********************************************************************************************************/
	// KcpPeer
	/***********************************************************************************************************/

	struct KcpPeer : Item, xx::TimeoutBase {
		// ep ������� timeout ������
		virtual TimeoutManager* GetTimeoutManager() override;

		// �����շ����ݵ����� udp peer
		Item_r<UdpListener> owner;

		// kcp ���������
		ikcpcb* kcp = nullptr;
		uint32_t conv = 0;
		int64_t createMS = 0;
		uint32_t nextUpdateMS = 0;

		// �� owner ���
		sockaddr_in6 addr;
		std::string ip;

		// ���������ڴ���������
		std::size_t readBufLen = 65536;

		// ��������
		xx::List<uint8_t> recv;

		// �ڲ�����������������ȷ�� conv �����
		int InitKcp();

		// ���ݽ����¼�: �� recv ������. Ĭ��ʵ��Ϊ echo
		virtual void OnReceive();

		// ������( ���ݿ�������һ����ȴ��ϲ� )
		int Send(uint8_t const* const& buf, ssize_t const& dataLen);

		// ���̷���, ֹͣ�ȴ�
		virtual void Flush();

		// �� ep ����.
		// ��֡ѭ������. ֡��Խ��, kcp ����Ч��Խ��. ���͵�Ƶ��Ϊ 100 fps
		virtual void UpdateKcpLogic(int64_t const& nowMS);

		// �� owner ����.
		// �����ݵ� kcp
		int Input(uint8_t* const& recvBuf, uint32_t const& recvLen);

		inline virtual void OnDisconnect(int const& reason) {};

		// ��ʱ��ɱ
		virtual void OnTimeout() override;

		~KcpPeer();
	};


	/***********************************************************************************************************/
	// KcpConn
	/***********************************************************************************************************/

	// todo: 


	/***********************************************************************************************************/
	// Context
	/***********************************************************************************************************/

	struct Context : TimeoutManager {
		// fd ������ ֮ Ψһ��������. ����� Item_r ��������
		inline static std::array<std::pair<std::unique_ptr<FDItem>, int64_t>, 40000> fdHandlers;

		// �ṩ�����汾�� for FDItem
		int64_t autoIncVersion = 0;

		// �� fd �� item ��Ψһ����������� Item_r ��������. �Դ������汾�Ź���
		ItemPool<std::unique_ptr<Item>> items;

		// kcp conv ֵ�� peer ��ӳ�䡣KcpPeer ����ʱ�Ӹ��ֵ��Ƴ� key
		xx::Dict<uint32_t, KcpPeer*> kcps;

		// ����ʱ��������Ϣ�ֵ� key: ip:port   value: conv, nowMS
		xx::Dict<std::string, std::pair<uint32_t, int64_t>> shakes;


		// epoll_wait �¼��洢
		std::array<epoll_event, 4096> events;

		// �洢�� epoll fd
		int efd = -1;

		// ����һЩ����ֵ�� int �ĺ���, ��������뽫����ڴ�
		int lastErrorNumber = 0;

		// ����buf for kcp read ��
		std::array<char, 65536> sharedBuf;

		// wheelLen: ��ʱ�����ӳߴ�( ��֡ )
		Context(std::size_t const& wheelLen = 1 << 12);

		virtual ~Context();


		// ���������� socket fd ������. < 0: error
		int MakeSocketFD(int const& port, int const& sockType = SOCK_STREAM); // SOCK_DGRAM

		// ��� fd �� epoll ����. return !0: error
		int Ctl(int const& fd, uint32_t const& flags, int const& op = EPOLL_CTL_ADD);

		// �رղ��� epoll �Ƴ�����
		int CloseDel(int const& fd);

		// ����һ�� epoll wait. �ɴ��볬ʱʱ��. 
		int Wait(int const& timeoutMS);

		// ���� kcp �� Update
		void UpdateKcps();

		/********************************************************/
		// �������ⲿ��Ҫʹ�õĺ���

		// ֡�߼����Ը����������. ���ط� 0 ���� Run �˳�. 
		inline virtual int Update() { return 0; }


		// ��ʼ���в�����ά����ָ��֡��. ��ʱ��������֡
		int Run(double const& frameRate = 60.3);

		// ���� ������	// todo: ֧����дip, ֧�ִ��븴�� fd
		template<typename T = TcpListener, typename ...Args>
		Item_r<T> CreateTcpListener(int const& port, Args&&... args);

		// ���� ���� peer
		template<typename T = TcpDialer, typename ...Args>
		Item_r<T> CreateTcpDialer(Args&&... args);

		// ���� timer
		template<typename T = Timer, typename ...Args>
		Item_r<T> CreateTimer(int const& interval, std::function<void(Timer* const& timer)>&& cb, Args&&...args);

		// ���� UdpPeer
		template<typename T = UdpPeer, typename ...Args>
		Item_r<T> CreateUdpPeer(int const& port, Args&&... args);
	};



	/***********************************************************************************************************/
	// Util funcs
	/***********************************************************************************************************/

	// ip, port תΪ addr
	int FillAddress(std::string const& ip, int const& port, sockaddr_in6& addr);
}
