#include "uv.h"
#include <chrono>
#include <cassert>
#include <iostream>
#include <cstring>
size_t counter = 0;

int Send(uv_stream_t* const& stream, char const* const& buf, ssize_t const& dataLen) {
	struct uv_write_t_ex : uv_write_t {
		uv_buf_t buf;
	};
	auto req = (uv_write_t_ex*)malloc(sizeof(uv_write_t_ex) + dataLen);
	memcpy(req + 1, buf, dataLen);
	req->buf.base = (char*)(req + 1);
	req->buf.len = decltype(uv_buf_t::len)(dataLen);
	return uv_write(req, stream, &req->buf, 1, [](uv_write_t *req, int status) {
		free(req);
		if (status) return;
	});
}
int main() {
	uv_loop_t uv;
	uv_loop_init(&uv);
	uv_tcp_t peer;
	uv_tcp_init(&uv, &peer);
	sockaddr_in addr;
	uv_ip4_addr("127.0.0.1", 12345, &addr);
	struct uv_connect_t_ex : uv_connect_t {
		uv_tcp_t* peer;
	};
	auto req = (uv_connect_t_ex*)malloc(sizeof(uv_connect_t_ex));
	req->peer = &peer;
	uv_tcp_connect(req, &peer, (sockaddr*)&addr, [](uv_connect_t* req, int status) {
		auto peer = ((uv_connect_t_ex*)req)->peer;
		free(req);
		if (status) {
			uv_close((uv_handle_t*)peer, nullptr);
			return;
		}
		uv_read_start((uv_stream_t*)peer, [](uv_handle_t* h, size_t suggested_size, uv_buf_t* buf) {
			buf->base = (char*)malloc(suggested_size);
			buf->len = decltype(uv_buf_t::len)(suggested_size);
		}, [](uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
			if (nread > 0) {
				Send(stream, buf->base, nread);
			}
			free(buf->base);
			if (nread < 0 || (counter += nread) > 100000) {
				auto h = (uv_handle_t*)stream;
				assert(!uv_is_closing(h));
				uv_close(h, nullptr);
			}
		});
		Send((uv_stream_t*)peer, "a", 1);
	});
	auto t = std::chrono::system_clock::now();
	uv_run(&uv, UV_RUN_DEFAULT);
	uv_loop_close(&uv);
	std::cout << double(std::chrono::nanoseconds(std::chrono::system_clock::now() - t).count()) / 1000000000 << std::endl;
	std::cin.get();
	return 0;
}
