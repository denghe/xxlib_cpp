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


// todo: ���к�����һ�� disposed ���?


// �ڴ�ģ��Ϊ Ԥ��������ָ��
// �Ӵ���������ʼ�ͷ��� ����, �ʲ���Ҫ��취�ӳ�
// ʡ���� lock() �����������¼��ɷ�Ч��, ��ʱ���� timer �����ӳ�ִ��һ�δ������ΪҲ�������
// ��֮������������ Ҫ�ͷű���ֱ�ӻ��ӵ� Dispose( close fd, �� epoll �Ƴ�, ���������Ƴ��Դ��� ���� )

// ע�⣺
// �����������޷�ִ�� shared_from_this
// ������������� Dispose �޷����� ������ override �Ĳ���, �����
// �ϲ���ֻ�����Լ�������, ����������ʱ�޷������ϲ����Ա�� override �ĺ���

namespace xx::Epoll {

	/***********************************************************************************************************/
	// Item
	/***********************************************************************************************************/

	struct Context;

	struct Item : std::enable_shared_from_this<Item> {
		// ָ��������
		Context* ep = nullptr;

		// �ڶ��󴴽��ɹ� & ��� & ���õ�λ ��ᱻִ��, �����Ա���һЩ���ڳ�ʼ��������ģ�⹹�캯��
		inline virtual void Init() {}

		// ���ٱ�־
		bool disposed = false;
		inline bool Disposed() const { return disposed; }

		// flag == -1: �����������е���.  0: �������ص�  1: �����ص�
		inline virtual void Dispose(int const& flag) {
			// ���д���Ϊ����ʾ��, ���㸴��С��

			// ��� & ���� �Ա����ظ�ִ�������߼�
			if (disposed) return;
			disposed = true;

			// �����������Լ�����������
			Disposing(flag);

			// ������� �������Ƴ��Ĵ��� �Դ�������
		};

		// ������
		inline virtual void Disposing(int const& flag) {}

		// ÿ�����������������Ҫд this->Dispose(-1);
		virtual ~Item() { this->Dispose(-1); }
	};


	/***********************************************************************************************************/
	// FDHandler
	/***********************************************************************************************************/

	struct FDHandler : Item {
		// linux ϵͳ�ļ�������. ͬʱҲ�� ep->fdHandlers ���±�
		int fd = -1;

		// epoll fd �¼�����. return �� 0 ��ʾ��ɱ
		virtual int OnEpollEvent(uint32_t const& e) = 0;

		// �� fd, �� epoll �Ƴ�, call Disposing, �������Ƴ�( ���ܴ������� )
		virtual void Dispose(int const& flag) override;

		virtual ~FDHandler() { this->Dispose(-1); }
	};


	/***********************************************************************************************************/
	// Timer
	/***********************************************************************************************************/

	struct Timer : Item, xx::TimeoutBase {
		// ep->timers ���±�
		int indexAtContainer = -1;

		// ʱ�䵽��ʱ����. �����ʵ�� repeat Ч��, ���ں�������ǰ �Լ� timer->SetTimeout
		std::function<void(Timer* const& timer)> onFire;

		// ���𴥷� onFire
		virtual void OnTimeout() override;

		// �� timeoutManager �Ƴ�, call Disposing, �������Ƴ�( ���ܴ������� )
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

		virtual int OnEpollEvent(uint32_t const& e) override;

		// ���ݽ����¼�: �� recv ������. Ĭ��ʵ��Ϊ echo
		virtual int OnReceive();

		// ���ڴ���ʱ����
		virtual void OnTimeout() override;

		// ����ʱ�Ĵ���
		inline virtual void OnDisconnect() {}

		// ���� OnDisconnect
		virtual void Disposing(int const& flag) override;

		~TcpPeer() { this->Dispose(-1); }

		// Buf ���������в���ʼ���͡������Ϣ��ο� Buf ���캯��
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
		// ���ǲ��ṩ���� peer �����ʵ��. ���� nullptr ��ʾ����ʧ��
		virtual std::shared_ptr<TcpPeer> OnCreatePeer();

		// ���ǲ��ṩΪ peer ���¼���ʵ��. ���ط� 0 ��ʾ��ֹ accept
		inline virtual int OnAccept(std::shared_ptr<TcpPeer>& peer) { return 0; }

		// ���� accept
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
		// �Ƿ����ӳɹ�
		bool connected = false;

		// �ɹ�, ��ʱ�����Ӵ��� ���������ú���. ��һ���ж� connected ��֪״̬
		inline virtual void OnConnect() {}

		// ͨ�� connected ��·�������¼��߼�
		virtual int OnEpollEvent(uint32_t const& e) override;

		// ���� OnConnect
		virtual void Disposing(int const& flag) override;

		~TcpConn() { this->Dispose(-1); }
	};


	/***********************************************************************************************************/
	// UdpPeer
	/***********************************************************************************************************/

	struct UdpPeer : FDHandler {
		// ��Ÿ� udp socket ռ�õ����ĸ����ض˿�	// todo: 0 ����Ӧ����Ҫȥ��ȡ
		int port = -1;

		// �������ݽ���
		virtual int OnEpollEvent(uint32_t const& e) override;

		// �������ݵ����¼�. Ĭ��ʵ��Ϊ echo. ʹ�� sendto �����յ�������.
		virtual int OnReceive(sockaddr* fromAddr, char const* const& buf, std::size_t const& len);

		// ֱ�ӷ�װ sendto ����
		int SendTo(sockaddr* toAddr, char const* const& buf, std::size_t const& len);

		~UdpPeer() { this->Dispose(-1); }
	};


	/***********************************************************************************************************/
	// UdpListener
	/***********************************************************************************************************/

	struct KcpPeer;
	struct UdpListener : UdpPeer {
		// todo: �Լ������շ�ģ������ ģ�� accept( ���Ѵ����� KcpPeer ������ )
		// todo: ѭ��ʹ��һ�� UdpPeer, �����߼� kcp ����. ��� UdpPeer ���ڼ��� epoll �¼�������� ����ƿ�� )
		// todo: ʵ�������߼�

		// ��������
		uint32_t convId = 0;

		// ���ֳ�ʱʱ��
		int handShakeTimeoutMS = 3000;

		// �ж��յ�����������, ģ�����֣� �������� KcpPeer
		virtual int OnReceive(sockaddr* fromAddr, char const* const& buf, std::size_t const& len);

		// ���ǲ��ṩ���� peer �����ʵ��. ���� nullptr ��ʾ����ʧ��
		virtual std::shared_ptr<KcpPeer> OnCreatePeer();

		// ���ǲ��ṩΪ peer ���¼���ʵ��. ���ط� 0 ��ʾ��ֹ accept
		inline virtual int OnAccept(std::shared_ptr<KcpPeer>& peer) { return 0; }
	};


	/***********************************************************************************************************/
	// KcpPeer
	/***********************************************************************************************************/

	struct KcpPeer : Item, xx::TimeoutBase {
		// �����շ����ݵ����� udp peer
		std::shared_ptr<UdpListener> owner;

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
		virtual int OnReceive();

		// ������( ���ݿ�������һ����ȴ��ϲ� )
		int Send(uint8_t const* const& buf, ssize_t const& dataLen);

		// ���̷���, ֹͣ�ȴ�
		virtual void Flush();

		// �� ep ����.
		// ��֡ѭ������. ֡��Խ��, kcp ����Ч��Խ��. ���͵�Ƶ��Ϊ 100 fps
		virtual int UpdateKcpLogic(int64_t const& nowMS);

		// �� owner ����.
		// �����ݵ� kcp
		int Input(uint8_t* const& recvBuf, uint32_t const& recvLen);

		// ��ʱ��ɱ
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
		// fd ������ ֮ Ψһ��������. �����þ����� weak_ptr
		std::vector<std::shared_ptr<FDHandler>> fdHandlers;

		// kcp conv ֵ�� peer ��ӳ��. ʹ�� xxDict ��Ϊ�˷������ kv ʱ value.Dispose(1) ֧�� kcp �Լ��� kcps �Ƴ�
		xx::Dict<uint32_t, std::shared_ptr<KcpPeer>> kcps;

		// ����ʱ��������Ϣ�ֵ� key: ip:port   value: conv, nowMS
		xx::Dict<std::string, std::pair<uint32_t, int64_t>> shakes;

		// timer Ψһ��������. �����þ����� weak_ptr
		std::vector<std::shared_ptr<Timer>> timers;

		// epoll_wait �¼��洢
		std::array<epoll_event, 4096> events;

		// �洢�� epoll fd
		int efd = -1;

		// ����һЩ����ֵ�� int �ĺ���, ��������뽫����ڴ�
		int lastErrorNumber = 0;

		// ��ʱ������
		xx::TimeoutManager timeoutManager;

		// ����buf for kcp read ��
		std::array<char, 65536> sharedBuf;

		// maxNumFD: fd ����
		Context(int const& maxNumFD = 65536);

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
		template<typename L = TcpListener, typename ...Args>
		std::shared_ptr<L> TcpListen(int const& port, Args&&... args);

		// ���� ���� peer
		template<typename C = TcpConn, typename ...Args>
		std::shared_ptr<C> TcpDial(char const* const& ip, int const& port, int const& timeoutInterval, Args&&... args);

		// ���� timer
		template<typename T = Timer, typename ...Args>
		std::shared_ptr<T> Delay(int const& interval, std::function<void(Timer* const& timer)>&& cb, Args&&...args);

		// ���� UdpPeer
		template<typename U = UdpPeer, typename ...Args>
		std::shared_ptr<U> UdpBind(int const& port, Args&&... args);
	};

}
