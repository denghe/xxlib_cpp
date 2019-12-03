#pragma once
#include <cstring>
#include <memory>
#include <type_traits>
#include <cassert>
#include <cstdint>
#include <functional>
namespace xx {
	template<typename Value, typename Size_t = int, typename Version_t = int64_t>
	struct ItemPool {
		struct Data {
			Version_t version;
			Value value;
			Size_t next;	// �����һ�� free �ռ���±�
		};

		// [ָ��, �汾��] ����
		Data* buf = nullptr;
		// buf ��
		Size_t cap = 0;
		// ��������( ����Ԫ�ظ��� )
		Size_t len = 0;

		// ���ɿռ�����ͷ
		Size_t freeHeader = -1;
		// ���ɿռ�����
		Size_t freeCount = 0;

		// �������. Add ʱ ++version ���
		Version_t version = 0;

		// ���㲢����Ԫ�ظ���
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

		// ��Ӳ����ش�ŵ��±꣬������ RemoveAt, At
		template<typename ...Args>
		Size_t Add(Args&&...args) {
			auto idx = Alloc();
			new (&buf[idx].value) Value(std::forward<Args>(args)...);
			buf[idx].version = ++version;
			buf[idx].next = -1;
			return idx;
		}
		template<typename ...Args>
		Size_t TryAdd(Args&&...args) {
			auto idx = Alloc();
			try {
				new (&buf[idx].value) Value(std::forward<Args>(args)...);
			}
			catch (...) {
				return -1;
			}
			return idx;
		}

		// ͨ���±��Ƴ�
		inline void RemoveAt(Size_t const& idx) noexcept {
			assert(idx < len);
			buf[idx].version = 0;
			buf[idx].next = freeHeader;			     // ָ�� ���ɽڵ�����ͷ
			if constexpr (!std::is_pod_v<Value>) {
				buf[idx].value.~Value();
			}
			freeHeader = idx;
			freeCount++;
		}

		// ��λ���洢��( ����д assert? )
		Data& At(Size_t const& idx) noexcept {
			assert(idx < len);
			return buf[idx];
		}

		// ��λ���洢��( ����д assert? )
		Value& ValueAt(Size_t const& idx) noexcept {
			assert(idx < len);
			return buf[idx].value;
		}

		// ��λ���洢��( ����д assert? )
		Version_t& VersionAt(Size_t const& idx) noexcept {
			assert(idx < len);
			return buf[idx].version;
		}

	protected:
		Size_t Alloc() {
			int idx;
			// ��� ���ɽڵ����� ����, ȡһ����������
			if (freeCount) {
				idx = freeHeader;
				freeHeader = (int)buf[idx].next;
				freeCount--;
			}
			else {
				// ���пսڵ㶼�ù���, ����
				if (len == cap) {
					buf = (Data*)realloc((void*)buf, sizeof(Data) * cap * 2);
				}
				// ָ�� Resize ����Ŀռ����
				idx = len;
				len++;
			}
			return idx;
		}
	};
}
