#pragma once
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <array>
#include <algorithm>
#ifdef _WIN32
#include <intrin.h>     // _BitScanReverse  64
#endif

namespace xx {

	/*
		lua sample:

		xx::MemPool lmp;
		...
		auto L = lua_newstate([](void *ud, void *ptr, std::size_t osize, std::size_t nsize)
		{
			return ((xx::MemPool*)ud)->Realloc(ptr, nsize, osize);
		}, &lmp);
	*/
	struct MemPool {
		static_assert(sizeof(std::size_t) == sizeof(void*), "");
		std::array<void*, sizeof(void*) * 8> headers;

		MemPool() {
			headers.fill(nullptr);
		}

		~MemPool() {
			for (auto header : headers) {
				while (header) {
					auto next = *(void**)header;
					free(header);
					header = next;
				}
			}
		}

		inline void* Alloc(std::size_t siz) {
			assert(siz);
			siz += sizeof(void*);			// avoid special space for store idx( after alloc ) or next ptr( after free )
			auto idx = Calc2n(siz);
			if (siz > (std::size_t(1) << idx)) {
				siz = std::size_t(1) << ++idx;	// align with 2^n
			}

			auto p = headers[idx];
			if (p) {
				headers[idx] = *(void**)p;	// link to next
			}
			else {
				p = malloc(siz);
			}
			if (!p) return nullptr;
			*(std::size_t*)p = idx;				// store idx at memblock's special space
			return (void**)p + 1;
		}

		inline void Free(void* p) {
			if (!p) return;
			p = (void**)p - 1;				// ref to special space for get idx back
			auto idx = *(std::size_t*)p;
			*(void**)p = headers[idx];		// store next to special space
			headers[idx] = p;				// link to ptr
		}

		inline void* Realloc(void* p, std::size_t const& newSize, std::size_t const& dataLen = -1) {
			if (!newSize) {
				Free(p);
				return nullptr;
			}
			if (!p) return Alloc(newSize);

			auto originalSize = (std::size_t)((std::size_t(1) << *(std::size_t*)((void**)p - 1)) - sizeof(void*));
			if (originalSize >= newSize) return p;

			auto np = Alloc(newSize);
			memcpy(np, p, std::min(originalSize, dataLen));
			Free(p);
			return np;
		}

		inline static std::size_t Calc2n(std::size_t n) {
			assert(n);
#ifdef _MSC_VER
			unsigned long r = 0;
#if defined(_WIN64) || defined(_M_X64)
			_BitScanReverse64(&r, n);
# else
			_BitScanReverse(&r, n);
# endif
			return (std::size_t)r;
#else
#if defined(__LP64__) || __WORDSIZE == 64
			return int(63 - __builtin_clzl(n));
# else
			return int(31 - __builtin_clz(n));
# endif
#endif
		}
	};
}
