#pragma once
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <functional>
#include <memory>
#include <ostream>
#include <tuple>
#include <utility>

#include "fcontext/fcontext.h"
#include <assert.h>

//#define BOOST_ASSERT_IS_VOID
namespace boost_context {

	struct forced_unwind {
		fcontext_t  fctx{ nullptr };
#ifndef BOOST_ASSERT_IS_VOID
		bool        caught{ false };
#endif

		forced_unwind() = default;

		forced_unwind(fcontext_t fctx_) :
			fctx(fctx_) {
		}

#ifndef BOOST_ASSERT_IS_VOID
		~forced_unwind() {
			assert(caught);
		}
#endif
	};

	inline
		fcontext_transfer_t context_unwind(fcontext_transfer_t t) {
		throw forced_unwind(t.ctx);
		return { nullptr, nullptr };
	}

	template< typename Rec >
	fcontext_transfer_t context_exit(fcontext_transfer_t t) noexcept {
		Rec * rec = static_cast<Rec *>(t.data);
		// destroy context stack
		rec->deallocate();
		return { nullptr, nullptr };
	}

	template< typename Rec >
	void context_entry(fcontext_transfer_t t) noexcept {
		// transfer control structure to the context-stack
		Rec * rec = static_cast<Rec *>(t.data);
		assert(nullptr != t.ctx);
		assert(nullptr != rec);
		try {
			// jump back to `create_context()`
			t = jump_fcontext(t.ctx, nullptr);
			// start executing
			t.ctx = rec->run(t.ctx);
		}
		catch (forced_unwind const& ex) {
			t = { ex.fctx, nullptr };
#ifndef BOOST_ASSERT_IS_VOID
			const_cast<forced_unwind &>(ex).caught = true;
#endif
		}
		assert(nullptr != t.ctx);
		// destroy context-stack of `this`context on next context
		ontop_fcontext(t.ctx, rec, context_exit< Rec >);
		//BOOST_ASSERT_MSG(false, "context already terminated");
	}

	template< typename Ctx, typename Fn >
	fcontext_transfer_t context_ontop(fcontext_transfer_t t) {
		auto p = static_cast<std::tuple< Fn > *>(t.data);
		assert(nullptr != p);
		typename std::decay< Fn >::type fn = std::get< 0 >(*p);
		t.data = nullptr;
		Ctx c{ t.ctx };
		// execute function, pass continuation via reference
		c = fn(std::move(c));
		return { std::exchange(c.fctx_, nullptr), nullptr };
	}

	template< typename Ctx, typename Fn >
	class record {
	private:
		fcontext_stack_t                                    sctx_;
		typename std::decay< Fn >::type                     fn_;

		static void destroy(record * p) noexcept {
			fcontext_stack_t sctx = p->sctx_;
			// deallocate record
			p->~record();
			// destroy stack with stack allocator
			destroy_fcontext_stack(&sctx);
		}

	public:
		record(fcontext_stack_t sctx,
			Fn && fn) noexcept :
			sctx_(sctx),
			fn_(std::forward< Fn >(fn)) {
		}

		record(record const&) = delete;
		record & operator=(record const&) = delete;

		void deallocate() noexcept {
			destroy(this);
		}

		fcontext_t run(fcontext_t fctx) {
			Ctx c{ fctx };
			// invoke context-function
			c = fn_(std::move(c));
			return std::exchange(c.fctx_, nullptr);
		}
	};

	template< typename Record, typename Fn >
	fcontext_t create_context1(Fn && fn, size_t const& stackSize) {
		auto sctx = create_fcontext_stack(stackSize);
		// reserve space for control structure
		void * storage = reinterpret_cast<void *>(
			(reinterpret_cast<uintptr_t>(sctx.sptr) - static_cast<uintptr_t>(sizeof(Record)))
			& ~static_cast<uintptr_t>(0xff));
		// placment new for control structure on context stack
		Record * record = new (storage) Record{
				sctx, std::forward< Fn >(fn) };
		// 64byte gab between control structure and stack top
		// should be 16byte aligned
		void * stack_top = reinterpret_cast<void *>(
			reinterpret_cast<uintptr_t>(storage) - static_cast<uintptr_t>(64));
		void * stack_bottom = reinterpret_cast<void *>(
			reinterpret_cast<uintptr_t>(sctx.sptr) - static_cast<uintptr_t>(sctx.ssize));
		// create fast-context
		const std::size_t size = reinterpret_cast<uintptr_t>(stack_top) - reinterpret_cast<uintptr_t>(stack_bottom);
		const fcontext_t fctx = make_fcontext(stack_top, size, &context_entry< Record >);
		assert(nullptr != fctx);
		// transfer control structure to context-stack
		return jump_fcontext(fctx, record).ctx;
	}

	class continuation {
	private:
		template< typename Ctx, typename Fn >
		friend class record;

		template< typename Ctx, typename Fn >
		friend fcontext_transfer_t
			context_ontop(fcontext_transfer_t);

		template<typename Fn >
		friend continuation
			callcc(Fn &&, size_t const& stackSize = 1024 * 1024);

		fcontext_t  fctx_{ nullptr };

		continuation(fcontext_t fctx) noexcept :
			fctx_{ fctx } {
		}

	public:
		continuation() noexcept = default;

		~continuation() {
			if (nullptr != fctx_) {
				ontop_fcontext(
					std::exchange(fctx_, nullptr),
					nullptr,
					context_unwind);
			}
		}

		continuation(continuation && other) noexcept {
			swap(other);
		}

		continuation & operator=(continuation && other) noexcept {
			if (this != &other) {
				continuation tmp = std::move(other);
				swap(tmp);
			}
			return *this;
		}

		continuation(continuation const& other) noexcept = delete;
		continuation & operator=(continuation const& other) noexcept = delete;

		continuation resume() & {
			return std::move(*this).resume();
		}

		continuation resume() && {
			assert(nullptr != fctx_);
			return { jump_fcontext(
						std::exchange(fctx_, nullptr),
						nullptr).ctx };
		}

		template< typename Fn >
		continuation resume_with(Fn && fn) & {
			return std::move(*this).resume_with(std::forward< Fn >(fn));
		}

		template< typename Fn >
		continuation resume_with(Fn && fn) && {
			assert(nullptr != fctx_);
			auto p = std::make_tuple(std::forward< Fn >(fn));
			return { ontop_fcontext(
						std::exchange(fctx_, nullptr),
						&p,
						context_ontop< continuation, Fn >).ctx };
		}

		explicit operator bool() const noexcept {
			return nullptr != fctx_;
		}

		bool operator!() const noexcept {
			return nullptr == fctx_;
		}

		bool operator<(continuation const& other) const noexcept {
			return fctx_ < other.fctx_;
		}

		template< typename charT, class traitsT >
		friend std::basic_ostream< charT, traitsT > &
			operator<<(std::basic_ostream< charT, traitsT > & os, continuation const& other) {
			if (nullptr != other.fctx_) {
				return os << other.fctx_;
			}
			else {
				return os << "{not-a-context}";
			}
		}

		void swap(continuation & other) noexcept {
			std::swap(fctx_, other.fctx_);
		}
	};

	template<
		typename Fn
	>
		continuation
		callcc(Fn && fn, size_t const& stackSize) {
		using Record = record< continuation, Fn >;
		return continuation{
					create_context1< Record >(
							std::forward< Fn >(fn), stackSize) }.resume();
	}


}









#include <vector>
#include <functional>
#include <stdint.h>
#include <assert.h>
#include <string.h>

namespace xx {
	struct Coro {
		boost_context::continuation& c;
		Coro(boost_context::continuation& c) : c(c) {}
		inline void operator()() {
			c = c.resume();
		}
	};
	struct Coros {
		Coros() {};

		// [](boost_context::continuation&& c) { ... c = c.resume(); ... return std::move(c); }
		template<typename Fn>
		inline void AddCore(Fn&& f) noexcept {
			cs.emplace_back(std::move(boost_context::callcc(std::move(f))));
		}

		// [](xx::Coro&& yield) {}
		template<typename Fn>
		inline void Add(Fn&& f) noexcept {
			AddCore([f = std::move(f)](boost_context::continuation&& c) {
				f(Coro(c));
				return std::move(c);
			});
		}

		inline size_t RunOnce() noexcept {
			if (!cs.size()) return 0;
			for (decltype(auto) i = cs.size() - 1; i != (size_t)-1; --i) {
				cs[i] = cs[i].resume();
				if (!cs[i]) {
					if (i + 1 < cs.size()) {
						std::swap(cs[i], cs[cs.size() - 1]);
					}
					cs.pop_back();
				}
			}
			return cs.size();
		}

	protected:
		std::vector<boost_context::continuation> cs;
	};
}
