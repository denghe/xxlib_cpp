#include "uv.h"
#include <cstdlib>
#include <cstring>
#include <cassert>

#define BLOCK_SIZE 65536

template<size_t blockSize = BLOCK_SIZE>
struct BlockPool {
	void* header = nullptr;
	inline void* Alloc() {
		if (!header) return ::malloc(blockSize);
		auto r = header;
		header = *(void**)header;
		return r;
	}
	inline void Free(void* p) {
		*(void**)p = header;
		header = p;
	}
	~BlockPool() {
		if (header) {
			::free(header);
			header = nullptr;
		}
	}
};
inline BlockPool<> bp;

struct uv_write_t_ex : uv_write_t {
	uv_buf_t buf;
};

int main() {
	uv_loop_t uv;
	uv_loop_init(&uv);
	uv_tcp_t listener;
	uv_tcp_init(&uv, &listener);
	sockaddr_in addr;
	uv_ip4_addr("0.0.0.0", 12345, &addr);
	uv_tcp_bind(&listener, (sockaddr*)&addr, 0);
	uv_listen((uv_stream_t*)&listener, 128, [](uv_stream_t* server, int status) {
		if (status) return;
		auto peer = (uv_tcp_t*)::malloc(sizeof(uv_tcp_t));
		uv_tcp_init(server->loop, peer);
		uv_accept(server, (uv_stream_t*)peer);
		uv_read_start((uv_stream_t*)peer, [](uv_handle_t* h, size_t suggested_size, uv_buf_t* buf) {
			assert(suggested_size <= BLOCK_SIZE);
			buf->base = (char*)bp.Alloc() + sizeof(uv_write_t_ex);			// 保留 uv_write_t_ex 空间以便直接用于 write
			buf->len = BLOCK_SIZE - sizeof(uv_write_t_ex);					// read 到的数据放在后面
		}, [](uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
			if (nread > 0) {
				auto req = (uv_write_t_ex*)(buf->base - sizeof(uv_write_t_ex));
				req->buf.base = buf->base;
				req->buf.len = nread;
				nread = uv_write(req, stream, &req->buf, 1, [](uv_write_t *req, int status) {
					bp.Free(req);
				});
			}
			else if (buf->base) {
				bp.Free(buf->base - sizeof(uv_write_t_ex));
			}
			if (nread < 0) {
				auto h = (uv_handle_t*)stream;
				assert(!uv_is_closing(h));
				uv_close(h, [](uv_handle_t* h) {
					::free(h);
				});
			}
		});
	});
	uv_run(&uv, UV_RUN_DEFAULT);
	uv_loop_close(&uv);
	return 0;
}
