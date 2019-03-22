#include <vector>
#include <functional>
#include <stdint.h>
#include <assert.h>
#ifdef _WIN32
#include <windows.h>
#else
#if defined(__APPLE__) && defined(__MACH__)
#define _XOPEN_SOURCE
#include <ucontext.h>
#else
#include <ucontext.h>
#endif
#endif

struct Coroutine;
using CoroutineFuncType = std::function<void(Coroutine& yield)>;
struct Coroutines;
struct Coroutine {
	Coroutines& owner;
	CoroutineFuncType func;
	Coroutine(Coroutines& owner, CoroutineFuncType&& func);
	Coroutine(Coroutine&& o);
	Coroutine& operator=(Coroutine&& o);
	~Coroutine();
	int resume();
	void operator()();	// yield
	operator bool();
#ifdef _WIN32
	LPVOID fiber = nullptr;
#else
	static void entry(uint32_t low32, uint32_t hi32);
	char* stack = nullptr;
	ucontext_t ctx;
#endif
};
struct Coroutines {
	std::vector<Coroutine> cors;
	size_t stackSize;
	Coroutines(size_t const& stackSize = 1024 * 1024);
	inline void Add(CoroutineFuncType&& func);
	size_t RunOnce();
#ifdef _WIN32
	LPVOID mainFiber = nullptr;
#else
	ucontext_t mainCtx;
#endif
};

/***************************************************************************************/
// Coroutine

inline Coroutine::Coroutine(Coroutines& owner, CoroutineFuncType&& func)
	: owner(owner)
	, func(std::move(func)) {
}

inline Coroutine::Coroutine(Coroutine&& o) 
	: owner(o.owner)
	, func(std::move(o.func))
#ifdef _WIN32
	, fiber(std::move(o.fiber))
#else
	, stack(std::move(o.stack))
	, ctx(std::move(o.ctx))
#endif
{
}
inline Coroutine& Coroutine::operator=(Coroutine&& o) {
	std::swap(owner, o.owner);
	std::swap(func, o.func);
#ifdef _WIN32
	std::swap(fiber, o.fiber);
#else
	std::swap(stack, o.stack);
	std::swap(ctx, o.ctx);
#endif
	return *this;
}

inline Coroutine::~Coroutine() {
#ifdef _WIN32
	if (fiber) {
		DeleteFiber(fiber);
		fiber = nullptr;
	}
#else
	if (stack) {
		delete[] stack;
		stack = nullptr;
	}
#endif
}

#ifdef _WIN32
#else
inline void Coroutine::entry(uint32_t low32, uint32_t hi32) {
	auto&& self = (Coroutine*)((uintptr_t)low32 | ((uintptr_t)hi32 << 32));
	self->func(*self);
	self->func = nullptr;
}
#endif

inline int Coroutine::resume() {
	if (!func) return -1;
#ifdef _WIN32
	if (!fiber) {
		fiber = CreateFiber(owner.stackSize, [](LPVOID lpParameter) {
			auto&& self = (Coroutine*)lpParameter;
			self->func(*self);
			self->func = nullptr;
			SwitchToFiber(self->owner.mainFiber);
		}, this);
	}
	SwitchToFiber(fiber);
#else
	getcontext(&ctx);
	stack = new char[owner.stackSize];
	ctx.uc_stack.ss_sp = stack;
	ctx.uc_stack.ss_size = owner.stackSize;
	ctx.uc_link = &owner.mainCtx;
	makecontext(&ctx, reinterpret_cast<void(*)(void)>(Coroutine::entry), 2, (uint32_t)(size_t)this, (uint32_t)((size_t)this >> 32));
	swapcontext(&owner.mainCtx, &ctx);
#endif
	return 0;
}

inline void Coroutine::operator()() {
#ifdef _WIN32
	SwitchToFiber(owner.mainFiber);
#else
#ifndef NDEBUG
	char stackChecker;
	assert(size_t(stack + owner.stackSize - &stackChecker) <= owner.stackSize);
#endif
	swapcontext(&ctx, &owner.mainCtx);
#endif
}

inline Coroutine::operator bool() {
	return (bool)func;
}

/***************************************************************************************/
// Coroutines

inline Coroutines::Coroutines(size_t const& stackSize)
	: stackSize(stackSize) {
#ifdef _WIN32
	mainFiber = ConvertThreadToFiber(nullptr);
#else
	memset(mainCtx, 0, sizeof(mainCtx));
	// todo: getcontext(mainCtx);
#endif
}

inline void Coroutines::Add(CoroutineFuncType&& func) {
	cors.emplace_back(*this, std::move(func));
}

inline size_t Coroutines::RunOnce() {
	if (!cors.size()) return 0;
	for (decltype(auto) i = cors.size() - 1; i != (size_t)-1; --i) {
		if (cors[i].resume()) {
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
