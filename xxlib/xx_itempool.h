#pragma once
#include <cstring>
#include <memory>
#include <type_traits>
#include <cassert>
#include <cstdint>
#include <functional>
namespace xx {
	template<typename Value, typename Size_t = std::size_t, typename Serial_t = int64_t>
	struct ItemPool {
		struct Data {
			Serial_t serial;
			Value value;
			Size_t next;	// 存放下一个 free 空间的下标
		};

		// [指针, 版本号] 数组
		Data* buf = nullptr;
		// buf 长
		Size_t cap = 0;
		// 数据区域长( 并非元素个数 )
		Size_t len = 0;

		// 自由空间链表头
		Size_t freeHeader = -1;
		// 自由空间链长
		Size_t freeCount = 0;

		// 自增序号. Add 时 ++serial 填充
		Serial_t serial = 0;

		// 计算并返回元素个数
		Size_t Count() {
			return len - freeCount;
		}

		ItemPool(Size_t const& cap = 8192)
			: cap(cap) {
			assert(cap);
			buf = (Data*)malloc(cap * sizeof(Data));
			memset((void*)buf, 0, cap * sizeof(Data));
		}

		~ItemPool() {
			Clear();
			free(buf);
			buf = nullptr;
		}

		void Clear(std::function<void(Size_t const& idx, Data & data)>&& cb = nullptr) noexcept {
			if (!len) return;
			for (Size_t i = 0; i < len; ++i) {
				if (buf[i].serial) {
					if (cb) {
						cb(i, buf[i]);
					}
					if constexpr (!std::is_pod_v<Value>) {
						buf[i].~Value();
					}
				}
			}
			freeHeader = -1;
			freeCount = 0;
			len = 0;
		}

		// 添加并返回存放点下标，可用于 RemoveAt, At
		template<typename ...Args>
		int Add(Args&&...args) {
			auto idx = Alloc();
			new (&buf[idx].value) Value(std::forward<Args>(args)...);
			buf[idx].serial = ++serial;
			buf[idx].next = -1;
			return idx;
		}
		template<typename ...Args>
		int TryAdd(Args&&...args) {
			auto idx = Alloc();
			try {
				new (&buf[idx].value) Value(std::forward<Args>(args)...);
			}
			catch (...) {
				return -1;
			}
			return idx;
		}

		// 通过下标移除
		inline void RemoveAt(Size_t const& idx) noexcept {
			assert(idx < len);
			buf[idx].serial = 0;
			buf[idx].next = freeHeader;			     // 指向 自由节点链表头
			if constexpr (!std::is_pod_v<Value>) {
				buf[idx].~Value();
			}
			freeHeader = idx;
			freeCount++;
		}

		// 定位到存储区( 方便写 assert? )
		Data& At(Size_t const& idx) noexcept {
			assert(idx < len);
			return buf[idx];
		}

	protected:
		Size_t Alloc() {
			int idx;
			// 如果 自由节点链表 不空, 取一个来当容器
			if (freeCount) {
				idx = freeHeader;
				freeHeader = (int)buf[idx].next;
				freeCount--;
			}
			else {
				// 所有空节点都用光了, 扩容
				if (len == cap) {
					buf = (Data*)realloc((void*)buf, sizeof(Data) * cap * 2);
				}
				// 指向 Resize 后面的空间起点
				idx = len;
				len++;
			}
			return idx;
		}
	};
}
