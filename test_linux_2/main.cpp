/*
	已知问题: 被客户端同时发起多个连接后 fd 似乎错乱了，telnet 连上来立刻断开，但是再继续开 telnet 连上来，之前看似断开的 telnet 似乎又恢复了

	// todo: 先网上找段经典的 c 版 epoll echo server 跑起来看看是否存在同样问题, 观察下
*/

#include "xx_epoll.h"

struct Server : xx::Epoll::Instance {
	Server() {
		coros.Add([this](xx::Coro& yield) {
			while (true) {
				xx::Cout(".");
				yield();
			}
		});
	}

	inline virtual void OnAccept(xx::Epoll::Peer_r pr, int const& listenIndex) override {
		assert(listenIndex >= 0);
		xx::CoutN(threadId, " OnAccept: listenIndex = ", listenIndex, ", id = ", pr->id, ", fd = ", pr->sockFD, ", ip = ", pr->ip);
	}

	inline virtual void OnDisconnect(xx::Epoll::Peer_r pr) override {
		xx::CoutN(threadId, " OnDisconnect: id = ", pr->id);
	}

	virtual int OnReceive(xx::Epoll::Peer_r pr) override {
		return pr->Send(xx::Epoll::Buf(pr->recv));		// echo
	}

};

int main(int argc, char* argv[]) {
	auto&& s = std::make_unique<Server>();
	int r = s->Listen(12345);
	assert(!r);
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
