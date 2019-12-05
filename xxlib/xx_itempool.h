#pragma once
#include "xx_object.h"
namespace xx {
	template<typename Value, typename Size_t = int, typename Version_t = int64_t>
	struct ItemPool {
		struct Data {
			Version_t version;
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

		// 自增序号. Add 时 ++version 填充
		Version_t version = 0;

		// 计算并返回元素个数
		Size_t Count() {
			return len - freeCount;
		}

		ItemPool(Size_t const& cap = 8192)
			: cap(cap) {
			assert(cap);
			buf = (Data*)malloc(cap * sizeof(Data));
		}

		~ItemPool() {
			Clear();
			free(buf);
			buf = nullptr;
		}

		void Clear(std::function<void(Size_t const& idx, Data & data)>&& cb = nullptr) noexcept {
			if (!len) return;
			for (Size_t i = 0; i < len; ++i) {
				if (buf[i].version) {
					if (cb) {
						cb(i, buf[i]);
					}
					if constexpr (!std::is_pod_v<Value>) {
						buf[i].value.~Value();
					}
				}
			}
			freeHeader = -1;
			freeCount = 0;
			len = 0;
		}

		// 添加并返回存放点下标，可用于 RemoveAt, At
		template<typename ...Args>
		Size_t Add(Args&&...args) {
			auto idx = Alloc();
			new (&buf[idx].value) Value(std::forward<Args>(args)...);
			buf[idx].version = ++version;
			buf[idx].next = -1;
			return idx;
		}

		//template<typename ...Args>
		//Size_t TryAdd(Args&&...args) {
		//	auto idx = Alloc();
		//	try {
		//		new (&buf[idx].value) Value(std::forward<Args>(args)...);
		//	}
		//	catch (...) {
		//		buf[idx].version = 0;
		//		buf[idx].next = freeHeader;			     // 指向 自由节点链表头
		//		freeHeader = idx;
		//		++freeCount;
		//		return -1;
		//	}
		//	buf[idx].version = ++version;
		//	buf[idx].next = -1;
		//	return idx;
		//}

		// 通过下标移除
		inline void RemoveAt(Size_t const& idx) noexcept {
			assert(idx < len);
			assert(buf[idx].next == -1);
			assert(buf[idx].version);
			buf[idx].version = 0;
			buf[idx].next = freeHeader;					// 指向 自由节点链表头
			if constexpr (!std::is_pod_v<Value>) {
				buf[idx].value.~Value();
			}
			freeHeader = idx;
			assert(freeHeader >= 0);
			assert(buf[freeHeader].version == 0);
			++freeCount;
		}

		// 定位到存储区( 方便写 assert? )
		Data& At(Size_t const& idx) noexcept {
			assert(idx < len);
			return buf[idx];
		}

		// 定位到存储区( 方便写 assert? )
		Value& ValueAt(Size_t const& idx) noexcept {
			assert(idx < len);
			return buf[idx].value;
		}

		// 定位到存储区( 方便写 assert? )
		Version_t& VersionAt(Size_t const& idx) noexcept {
			assert(idx < len);
			return buf[idx].version;
		}

	protected:
		Size_t Alloc() {
			Size_t idx;
			// 如果 自由节点链表 不空, 取一个来当容器
			if (freeCount) {
				assert(freeHeader >= 0);
				assert(buf[freeHeader].version == 0);
				idx = freeHeader;
				freeHeader = buf[idx].next;
				--freeCount;
			}
			else {
				// 所有空节点都用光了, 扩容
				if (len == cap) {
					if constexpr (IsTrivial_v<Value>) {
						buf = (Data*)realloc((void*)buf, sizeof(Data) * cap * 2);
					}
					else {
						auto newBuf = (Data*)malloc(sizeof(Data) * cap * 2);
						for (int i = 0; i < len; ++i) {
							new (&newBuf[i].value) Value((Value&&)buf[i].value);
							if constexpr (!std::is_pod_v<Value>) {
								buf[i].value.Value::~Value();
							}
						}
						free(buf);
						buf = newBuf;
					}
				}
				// 指向 Resize 后面的空间起点
				idx = len;
				++len;
			}
			return idx;
		}
	};
}
