#include "xx_epoll.h"
#include "xx_threadpool.h"

struct Server : xx::Epoll::Instance {
	inline virtual void OnAccept(xx::Epoll::Peer_r pr, int const& listenIndex) override {
		assert(listenIndex >= 0);
		xx::CoutN(threadId, " OnAccept: listenIndex = ", listenIndex, ", id = ", pr->id, ", fd = ", pr->sockFD, ", ip = ", pr->ip);
	}

	inline virtual void OnDisconnect(xx::Epoll::Peer_r pr) override {
		xx::CoutN(threadId, " OnDisconnect: id = ", pr->id);
	}

	// 线程池
	xx::ThreadPool tp = xx::ThreadPool(400);

	// 模拟收到数据后投递到线程池处理
	virtual int OnReceive(xx::Epoll::Peer_r pr) override {
		// 用智能指针包裹数据, 确保跨线程 lambda 捕获 引用计数正确
		auto&& buf = xx::Make<xx::Epoll::Buf>(pr->recv.buf, pr->recv.len);

		// 往线程池压入处理函数
		return tp.Add([this, pr, buf] {

			// 模拟一个长时间的处理. 1ms
			usleep(1000);

			// 将处理结果通过 epoll 线程发回
			Dispatch([this, pr, buf] {

				// 有可能等待处理期间 pr 已经断开了. 故用前判断一下
				if (pr) {

					// 发送
					pr->Send(std::move(*buf));
				}
			});
		});
	}

	Server() {
		// 通过协程, 每帧输出一个点
		coros.Add([this](xx::Coro& yield) {
			while (true) {
				xx::Cout(".");
				yield();
			}
		});
	}
};

int main(int argc, char* argv[]) {
	auto&& s = std::make_unique<Server>();
	int r = s->Listen(12345);
	assert(!r);

	xx::CoutN("thread:", 0);
	auto fd = s->listenFDs[0];
	std::vector<std::thread> threads;
	for (int i = 0; i < 2; ++i) {
		threads.emplace_back([fd, i] {
			auto&& s = std::make_unique<Server>();
			int r = s->ListenFD(fd);
			assert(!r);
			s->threadId = i + 1;
			xx::CoutN("thread:", i + 1);
			s->Run(1);
			}).detach();
	}

	// 按帧数为 1 的速度开始执行
	return s->Run(1);
}















































//#include "uv.h"
//#include <cstdlib>
//#include <cstring>
//#include <cassert>
//
//#define BLOCK_SIZE 65536
//
//template<size_t blockSize = BLOCK_SIZE>
//struct BlockPool {
//	void* header = nullptr;
//	inline void* Alloc() {
//		if (!header) return ::malloc(blockSize);
//		auto r = header;
//		header = *(void**)header;
//		return r;
//	}
//	inline void Free(void* p) {
//		*(void**)p = header;
//		header = p;
//	}
//	~BlockPool() {
//		if (header) {
//			::free(header);
//			header = nullptr;
//		}
//	}
//};
//inline BlockPool<> bp;
//
//struct uv_write_t_ex : uv_write_t {
//	uv_buf_t buf;
//};
//
//int main() {
//	printf("libuv echo server port 12345\n");
//
//	uv_loop_t uv;
//	uv_loop_init(&uv);
//	uv_tcp_t listener;
//	uv_tcp_init(&uv, &listener);
//	sockaddr_in addr;
//	uv_ip4_addr("0.0.0.0", 12345, &addr);
//	uv_tcp_bind(&listener, (sockaddr*)& addr, 0);
//	uv_listen((uv_stream_t*)& listener, 128, [](uv_stream_t* server, int status) {
//		if (status) return;
//		auto peer = (uv_tcp_t*)::malloc(sizeof(uv_tcp_t));
//		uv_tcp_init(server->loop, peer);
//		uv_accept(server, (uv_stream_t*)peer);
//		uv_read_start((uv_stream_t*)peer, [](uv_handle_t* h, size_t suggested_size, uv_buf_t* buf) {
//			assert(suggested_size <= BLOCK_SIZE);
//			buf->base = (char*)bp.Alloc() + sizeof(uv_write_t_ex);			// 保留 uv_write_t_ex 空间以便直接用于 write
//			buf->len = BLOCK_SIZE - sizeof(uv_write_t_ex);					// read 到的数据放在后面
//			}, [](uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
//				if (nread > 0) {
//					auto req = (uv_write_t_ex*)(buf->base - sizeof(uv_write_t_ex));
//					req->buf.base = buf->base;
//					req->buf.len = nread;
//					nread = uv_write(req, stream, &req->buf, 1, [](uv_write_t* req, int status) {
//						bp.Free(req);
//						});
//				}
//				else if (buf->base) {
//					bp.Free(buf->base - sizeof(uv_write_t_ex));
//				}
//				if (nread < 0) {
//					auto h = (uv_handle_t*)stream;
//					assert(!uv_is_closing(h));
//					uv_close(h, [](uv_handle_t* h) {
//						::free(h);
//						});
//				}
//			});
//		});
//	uv_run(&uv, UV_RUN_DEFAULT);
//	uv_loop_close(&uv);
//	return 0;
//}
