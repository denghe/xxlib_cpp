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

// todo: ��һ�� noexcept, const ɶ��

// �ڴ�ģ��Ϊ ����ָ�� �Ӵ���������ʼ�ͷ��� ����, ����������, �ʲ���Ҫ��취�ӳ�, Ҳ�����������¼��ɷ�Ч��( ʡ���� lock() )
// Dispose ��Ҫ���ã�close fd, �� epoll �Ƴ�, ���������Ƴ�
// ע�⣺
// �������� Dispose ����������� ���󴴽���һ�� �����쳣, ���Ҵ�ʱ�޷����� ������ override �Ĳ���, ����Ҫ�����ж�. ����Ϊһ������λ������������Ҳ����ִ���Ƴ���ش���
// Disposing ���������� �ӷ���������Դͷִ�� �����������ӵ���������
// ע��2��
// ��������������ĵ��ã��޷�ִ�� shared_from_this

namespace xx::Epoll {

	/***********************************************************************************************************/
	// Item
	/***********************************************************************************************************/

	struct Context;

	struct Item : std::enable_shared_from_this<Item> {
		// ָ��������
		Context* ep = nullptr;

		// ���ٱ�־
		bool disposed = false;
		inline bool Disposed() const noexcept {
			return disposed;
		}

		// flag == -1: �����������е���.  0: �������ص�  1: �����ص�
		inline virtual void Dispose(int const& flag) noexcept {
			// ���д���Ϊ����ʾ��, ���㸴��С��

			// ��� & ���� �Ա����ظ�ִ�������߼�
			if (disposed) return;
			disposed = true;

			// �����������Լ�����������
			Disposing(flag);

			// ������� �������Ƴ��Ĵ��� �Դ�������
		};

		// ������
		inline virtual void Disposing(int const& flag) noexcept {}

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
		virtual void Dispose(int const& flag) noexcept override;

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
		virtual void OnTimeout() noexcept override;

		// �� timeoutManager �Ƴ�, call Disposing, �������Ƴ�( ���ܴ������� )
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

		// writev ���� (buf + len) ���� ���������������鳤��
		static const std::size_t maxNumIovecs = 1024;

		virtual int OnEpollEvent(uint32_t const& e) override;

		// ���ݽ����¼�: �� recv ������
		virtual int OnReceive();

		// ���ڴ���ʱ����
		virtual void OnTimeout() override;

		// ����ʱ�Ĵ���
		inline virtual void OnDisconnect() {}

		// ���� OnDisconnect
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
		virtual void Disposing(int const& flag) noexcept override;

		~TcpConn() { this->Dispose(-1); }
	};


	/***********************************************************************************************************/
	// UdpPeer
	/***********************************************************************************************************/

	struct UdpPeer : FDHandler {
		// todo: �ṩ udp �����շ�����
		~UdpPeer() { this->Dispose(-1); }
	};


	/***********************************************************************************************************/
	// UdpListener
	/***********************************************************************************************************/

	struct UdpListener : UdpPeer {
		// todo: �Լ������շ�ģ������ ģ�� accept( ���Ѵ����� KcpPeer ������ )
		// ѭ��ʹ��һ�� UdpPeer, �����߼� kcp ����. ��� UdpPeer ���ڼ��� epoll �¼�������� ����ƿ�� )
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
		// fd ������ ֮ Ψһ��������. �����þ����� weak_ptr
		std::vector<std::shared_ptr<FDHandler>> fdHandlers;

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

		// maxNumFD: fd ����
		Context(int const& maxNumFD = 65536);

		~Context();

		// ���������� tcp fd ������. < 0: error
		static int MakeListenFD(int const& port);

		// ��� fd �� epoll ����. return !0: error
		int Ctl(int const& fd, uint32_t const& flags, int const& op = EPOLL_CTL_ADD);

		// �رղ��� epoll �Ƴ�����
		int CloseDel(int const& fd);

		// ����һ�� epoll wait. �ɴ��볬ʱʱ��. 
		int Wait(int const& timeoutMS);


		/********************************************************/
		// �������ⲿ��Ҫʹ�õĺ���

		// ֡�߼����Ը����������. ���ط� 0 ���� Run �˳�. 
		inline virtual int Update() { return 0; }

		// ��ʼ���в�����ά����ָ��֡��. ��ʱ��������֡
		int Run(double const& frameRate = 60.3);

		// ���� ������
		template<typename L = TcpListener, typename ...Args>
		inline std::shared_ptr<L> TcpListen(int const& port, Args&&... args);

		// ���� ���� peer
		template<typename C = TcpConn, typename ...Args>
		inline std::shared_ptr<C> TcpDial(char const* const& ip, int const& port, int const& timeoutInterval, Args&&... args);

		// ���� timer
		template<typename T = Timer, typename ...Args>
		inline std::shared_ptr<T> Delay(int const& interval, std::function<void(Timer* const& timer)>&& cb, Args&&...args);
	};

}
