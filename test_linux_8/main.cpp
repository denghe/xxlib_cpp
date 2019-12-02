#include "xx_itempool.h"
#include "xx_object.h"

struct Base {
};

namespace xx {
	template<>
	struct SFuncs<::Base*, void> {
		static inline void WriteTo(std::string& s, ::Base* const& in) noexcept {
			if (in) {
				xx::Append(s, "{this : ",(std::size_t)in, "}");
			}
			else {
				s.append("nil");
			}
		}
	};
}

// todo: 从老代码抠 weak_ptr. 存储 ItemPool*, index, serial

//template<typename Item>
//struct Item_r {
//	Item* item = nullptr;
//	uint64_t id = 0;
//
//	Peer_r(Item& item)
//		: item(&item)
//		, id(item.id) {
//	}
//
//	void Reset(Item& item) {
//		this->item = &item;
//		this->id = item.id;
//	}
//
//	Peer_r() = default;
//	Peer_r(Peer_r const&) = default;
//	Peer_r& operator=(Peer_r const&) = default;
//
//	inline operator bool() const {
//		return item->id == id;
//	}
//	Item* operator->() const {
//		return item;
//	}
//	Item& operator*() const {
//		return *item;
//	}
//
//	// todo: more func forward for easy use
//};

int main(int argc, char** argv) {
	xx::ItemPool<Base*> ip;

	auto b1 = new Base();
	auto idx = ip.Add(b1);
	xx::CoutN(ip.At(idx).next);
	xx::CoutN(ip.At(idx).serial);
	xx::CoutN(ip.At(idx).value);

	return 0;
}


// 下面代码展示一种 try 空指针的方式
//
//template<typename T>
//struct Ptr {
//	T* ptr = nullptr;
//	int lineNumber = -1;
//	T* operator->() {
//		if (!ptr) throw lineNumber;
//		return ptr;
//	}
//	void Clear(int const& lineNumber) {
//		if (ptr) {
//			delete ptr;
//			ptr = nullptr;
//			this->lineNumber = lineNumber;
//		}
//	}
//	void Reset(T* const& ptr, int const& lineNumber) {
//		Clear(lineNumber);
//		this->ptr = ptr;
//	}
//};
//
//#define PtrReset(self, ptr) self.Reset(nullptr, __LINE__);
//#define PtrClear(self) self.Clear(__LINE__);
//
//struct Foo {
//	int n = 0;
//	bool disposed = false;
//};
//
//int main(int argc, char** argv) {
//
//	Foo* f = new Foo;
//	f->n = 123;
//	Ptr<Foo> p;
//	p.ptr = f;
//	try {
//		xx::CoutN(p->n);
//		PtrClear(p);
//		xx::CoutN(p->n);
//	}
//	catch (int const& lineNumber) {
//		std::cout << lineNumber << std::endl;
//	}
//	catch (std::exception const& e) {
//		std::cout << e.what() << std::endl;
//	}
//}