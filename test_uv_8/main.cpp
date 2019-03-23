#include <vector>
#include <functional>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include "fcontext/fcontext.h"

struct Coroutine;
using CoroutineFunc = std::function<void(Coroutine& yield)>;
struct Coroutines;
struct Coroutine {
	Coroutine(Coroutines& owner, CoroutineFunc&& f, size_t const& stackSize = 0);
	Coroutine(Coroutine const&) = delete;
	Coroutine(Coroutine&& o);
	Coroutine& operator=(Coroutine const& o) = delete;
	Coroutine& operator=(Coroutine&& o);
	~Coroutine();

	void operator()();			// yield

	friend Coroutines;
protected:
	void Resume();
	operator bool();			// check alive

	Coroutines& owner;
	CoroutineFunc f;			// null: finish

	fcontext_stack_t s;
	fcontext_transfer_t t;		// for resume
	fcontext_transfer_t y;		// for yield
};
struct Coroutines {
	Coroutines(size_t const& defaultStackSize = 0);
	inline void Add(CoroutineFunc&& f, size_t const& stackSize = 0);
	size_t RunOnce();

	friend Coroutine;
protected:
	std::vector<Coroutine> cors;
	size_t stackSize;
};

/***************************************************************************************/
// Coroutine

inline Coroutine::Coroutine(Coroutines& owner, CoroutineFunc&& f, size_t const& stackSize)
	: owner(owner)
	, f(std::move(f)) {
	s = create_fcontext_stack(stackSize ? stackSize : owner.stackSize);
	t.ctx = make_fcontext(s.sptr, s.ssize, [](fcontext_transfer_t transfer) {
		auto&& self = *(Coroutine*)transfer.data;
		self.y = transfer;
		self.f(self);
		self.f = nullptr;
		self();
	});
	t.data = nullptr;			// for first time resume check
}

inline Coroutine::Coroutine(Coroutine&& o)
	: owner(o.owner)
	, f(std::move(o.f))
	, s(o.s)
	, t(o.t)
	, y(o.y)
{
	o.s.sptr = nullptr;
}

inline Coroutine& Coroutine::operator=(Coroutine&& o) {
	std::swap(owner, o.owner);
	std::swap(f, o.f);
	std::swap(s, o.s);
	std::swap(t, o.t);
	std::swap(y, o.y);
	return *this;
}

inline Coroutine::~Coroutine() {
	if (s.sptr) {
		destroy_fcontext_stack(&s);
	}
}

inline void Coroutine::Resume() {
	auto&& tmp = jump_fcontext(t.ctx, this);
	if (!t.data) {	// likely
		t = tmp;
	}
}

inline void Coroutine::operator()() {
	jump_fcontext(y.ctx, y.data);
}

inline Coroutine::operator bool() {
	return (bool)f;
}

/***************************************************************************************/
// Coroutines

inline Coroutines::Coroutines(size_t const& stackSize)
	: stackSize(stackSize) {
}

inline void Coroutines::Add(CoroutineFunc&& f, size_t const& stackSize) {
	cors.emplace_back(*this, std::move(f), stackSize);
}

inline size_t Coroutines::RunOnce() {
	if (!cors.size()) return 0;
	for (decltype(auto) i = cors.size() - 1; i != (size_t)-1; --i) {
		cors[i].Resume();
		if (!cors[i]) {
			if (i + 1 < cors.size()) {
				cors[i] = std::move(cors[cors.size() - 1]);
			}
			cors.pop_back();
		}
	}
	return cors.size();
}



#include <iostream>
#include <chrono>

int main(int argc, char* argv[]) {
	Coroutines cors;
	int i = 0;
	cors.Add([&](Coroutine& yield) {
		while (++i < 10000000) {
			yield();
		}
	});
	auto&& t = std::chrono::system_clock::now();
	while (cors.RunOnce()) {}
	std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - t).count() << std::endl;
	return 0;
}







//void fiber1_fn(fcontext_transfer_t transfer) {
//	auto&& t = std::chrono::system_clock::now();
//	for (int i = 0; i < 10000000; ++i) {
//		jump_fcontext(transfer.ctx, transfer.data);	// yield
//	}
//	std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - t).count() << std::endl;
//	jump_fcontext(transfer.ctx,transfer.data);	// end
//}

	//auto stack1 = create_fcontext_stack(80 * 1024);
	//auto fiber = make_fcontext(stack1.sptr, stack1.ssize, fiber1_fn);
	//auto t = jump_fcontext(fiber, NULL);
	//for (int i = 0; i < 10000000; ++i) {
	//	jump_fcontext(t.ctx, t.data);
	//}
	//destroy_fcontext_stack(&stack1);
