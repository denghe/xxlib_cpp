#pragma once
#include "xx_object.h"
namespace xx {

	// �������飬�����󲻶ϲ���ɾ��ʱ, ���±겢���ᷢ���ı�. �б��ڻ�������Ľ���ɾ��.
	// ������ͨ������ʵ��������ָ��. ͨ�� ItemPool*, version, index ���ж϶����Ƿ���Ч
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
		}

		~ItemPool() {
			Clear();
			free(buf);
			buf = nullptr;
		}

		// ��������Ѵ��ڵĶ��󡣿ɴ���Ԥ��������
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

		// ͨ���±��Ƴ�
		inline void RemoveAt(Size_t const& idx) noexcept {
			assert(idx < len);
			assert(buf[idx].next == -1);
			assert(buf[idx].version);
			buf[idx].version = 0;
			if constexpr (!std::is_pod_v<Value>) {
				buf[idx].value.~Value();
			}
			buf[idx].next = freeHeader;					// ָ�� ���ɽڵ�����ͷ
			freeHeader = idx;
			assert(freeHeader >= 0);
			assert(buf[freeHeader].version == 0);
			++freeCount;
		}

		// ��λ���洢��
		Data& At(Size_t const& idx) noexcept {
			assert(idx < len);
			return buf[idx];
		}

		// ��λ�� value
		Value& ValueAt(Size_t const& idx) noexcept {
			assert(idx < len);
			return buf[idx].value;
		}

		// ��λ�� version
		Version_t& VersionAt(Size_t const& idx) noexcept {
			assert(idx < len);
			return buf[idx].version;
		}

	protected:
		Size_t Alloc() {
			Size_t idx;
			// ��� ���ɽڵ����� ����, ȡһ����������
			if (freeCount) {
				assert(freeHeader >= 0);
				assert(buf[freeHeader].version == 0);
				idx = freeHeader;
				freeHeader = buf[idx].next;
				--freeCount;
			}
			else {
				// ���пսڵ㶼�ù���, ����
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
				// ָ�� Resize ����Ŀռ����
				idx = len;
				++len;
			}
			return idx;
		}
	};
}
