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

	void operator()();	// yield run
	operator bool();	// true: not finish

	friend Coroutines;
protected:
	Coroutines& owner;
	CoroutineFunc f;			// null: finish
	fcontext_stack_t s;
	fcontext_transfer_t t;
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
		self.t = transfer;
		self.f(self);
		self.f = nullptr;
		self();
	});
}

inline Coroutine::Coroutine(Coroutine&& o)
	: owner(o.owner)
	, f(std::move(o.f))
	, s(o.s)
	, t(o.t) {
	o.s.sptr = nullptr;
}

inline Coroutine& Coroutine::operator=(Coroutine&& o) {
	std::swap(owner, o.owner);
	std::swap(f, o.f);
	std::swap(s, o.s);
	std::swap(t, o.t);
	return *this;
}

inline Coroutine::~Coroutine() {
	if (s.sptr) {
		destroy_fcontext_stack(&s);
	}
}

inline void Coroutine::operator()() {
	t = jump_fcontext(t.ctx, this);
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
		cors[i]();
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

int main(int argc, char* argv[]) {
	Coroutines cors;
	cors.Add([](Coroutine& yield) {
		for (size_t i = 0; i < 9; i++) {
			std::cout << i << std::endl;
			yield();
		}
	});
	cors.Add([](Coroutine& yield) {
		for (size_t i = 11; i < 19; i++) {
			std::cout << i << std::endl;
			yield();
		}
	});
	while (cors.RunOnce()) {}
	return 0;
}
