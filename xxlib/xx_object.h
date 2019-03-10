#pragma once
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <unordered_map>
#include <array>
#include <type_traits>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <algorithm>
#include <limits>
#include <vector>
#include <deque>
#include <mutex>
#include <string>
#include <initializer_list>
#include <memory>
#include <chrono>
#include <iostream>

#include "fixed_function.hpp"

// 当 IOS 最低版本兼容参数低于 11 时无法启用 C++17, 故启用 C++14 结合下面的各种造假来解决
#ifdef __IPHONE_OS_VERSION_MIN_REQUIRED
#include <experimental/optional>
namespace std
{
	template<typename T>
	using optional = std::experimental::optional<T>;
}
#if __IPHONE_OS_VERSION_MIN_REQUIRED < 110000
namespace std
{
	template<class B, class D>
	inline constexpr bool is_base_of_v = is_base_of<B, D>::value;
	template<class T>
	inline constexpr bool is_arithmetic_v = is_arithmetic<T>::value;
	template<class T>
	inline constexpr bool is_floating_point_v = is_floating_point<T>::value;
	template<class T>
	inline constexpr bool is_integral_v = is_integral<T>::value;
	template<class T>
	inline constexpr bool is_unsigned_v = is_unsigned<T>::value;
	template<class T>
	inline constexpr bool is_enum_v = is_enum<T>::value;
	template<class T>
	inline constexpr bool is_pointer_v = is_pointer<T>::value;
	template<class T1, class T2>
	inline constexpr bool is_same_v = is_same<T1, T2>::value;

	template<class MutexType>
	class scoped_lock
	{
	public:
		explicit scoped_lock(MutexType& m) : m(m) { m.lock(); }
		~scoped_lock() { m.unlock(); }
		scoped_lock(const scoped_lock&) = delete;
		scoped_lock& operator=(const scoped_lock&) = delete;
	private:
		MutexType& m;
	};
}
#endif
#else
#include <optional>
#endif

#ifdef _WIN32
#include <intrin.h>     // _BitScanReverse  64
#include <objbase.h>
#endif

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#ifndef _countof
template<typename T, size_t N>
size_t _countof_helper(T const (&arr)[N])
{
	return N;
}
#define _countof(_Array) _countof_helper(_Array)
#endif

#ifndef _offsetof
#define _offsetof(s,m) ((size_t)&reinterpret_cast<char const volatile&>((((s*)0)->m)))
#endif

#ifndef container_of
#define container_of(ptr, type, member) \
  ((type *) ((char *) (ptr) - _offsetof(type, member)))
#endif


#if defined _MSC_VER
#define XX_SSCANF sscanf_s;
#else
#define XX_SSCANF sscanf;
#endif


#ifdef __ANDROID_NDK__
extern void uuid_generate(unsigned char* buf);
#else
#ifndef _WIN32
#include <uuid/uuid.h>
#endif
#endif


namespace std {
	using string_s = shared_ptr<string>;
	using string_w = weak_ptr<string>;
}

namespace xx {
	struct BBuffer;

	struct Object {
		virtual ~Object() {}

		inline virtual uint16_t GetTypeId() const noexcept { return 0; }
		inline virtual void ToBBuffer(BBuffer& bb) const noexcept {}
		inline virtual int FromBBuffer(BBuffer& bb) noexcept { return 0; }

		inline virtual void ToString(std::string& s) const noexcept {};
		inline virtual void ToStringCore(std::string& s) const noexcept {};

		bool toStringFlag = false;
		inline void SetToStringFlag(bool const& b = true) const noexcept {
			((Object*)this)->toStringFlag = b;
		}
	};

	using Object_s = std::shared_ptr<Object>;

	// TypeId 映射
	template<typename T>
	struct TypeId {
		static const uint16_t value = 0;
	};

	template<typename T>
	constexpr uint16_t TypeId_v = TypeId<T>::value;


	// 序列化 基础适配模板
	template<typename T, typename ENABLED = void>
	struct BFuncs {
		static inline void WriteTo(BBuffer& bb, T const& in) noexcept {
			assert(false);
		}
		static inline int ReadFrom(BBuffer& bb, T& out) noexcept {
			assert(false);
			return 0;
		}
	};

	// 字符串 基础适配模板
	template<typename T, typename ENABLED = void>
	struct SFuncs {
		static inline void WriteTo(std::string& s, T const& in) noexcept {
			assert(false);
		}
	};

	// for easy use
	template<typename T>
	void AppendCore(std::string& s, T const& v) {
		SFuncs<T>::WriteTo(s, v);
	}

	template<typename ...Args>
	void Append(std::string& s, Args const& ... args) {
		std::initializer_list<int> n{ ((AppendCore(s, args)), 0)... };
		(void)(n);
	}

	// 适配 char* \0 结尾 字串( 不是很高效 )
	template<>
	struct SFuncs<char*, void> {
		static inline void WriteTo(std::string& s, char* const& in) noexcept {
			if (in) {
				s.append(in);
			};
		}
	};

	// 适配 char const* \0 结尾 字串( 不是很高效 )
	template<>
	struct SFuncs<char const*, void> {
		static inline void WriteTo(std::string& s, char const* const& in) noexcept {
			if (in) {
				s.append(in);
			};
		}
	};

	// 适配 literal char[len] string
	template<size_t len>
	struct SFuncs<char[len], void> {
		static inline void WriteTo(std::string& s, char const(&in)[len]) noexcept {
			s.append(in);
		}
	};

	// 适配所有数字
	template<typename T>
	struct SFuncs<T, std::enable_if_t<std::is_arithmetic_v<T>>> {
		static inline void WriteTo(std::string& s, T const &in) noexcept {
			if constexpr (std::is_same_v<bool, T>) {
				s.append(in ? "true" : "false");
			}
			else if constexpr (std::is_same_v<char, T>) {
				s.append(in);
			}
			else {
				s.append(std::to_string(in));
			}
		}
	};

	// 适配 enum( 根据原始数据类型调上面的适配 )
	template<typename T>
	struct SFuncs<T, std::enable_if_t<std::is_enum_v<T>>> {
		static inline void WriteTo(std::string& s, T const &in) noexcept {
			s.append(std::to_string((std::underlying_type_t<T>)in));
		}
	};

	// 适配 Object
	template<typename T>
	struct SFuncs<T, std::enable_if_t<std::is_base_of_v<Object, T>>> {
		static inline void WriteTo(std::string& s, T const &in) noexcept {
			in.ToString(s);
		}
	};

	// 适配 std::string
	template<typename T>
	struct SFuncs<T, std::enable_if_t<std::is_base_of_v<std::string, T>>> {
		static inline void WriteTo(std::string& s, T const &in) noexcept {
			s.append(in);
		}
	};

	// 适配 std::shared_ptr<T>
	template<typename T>
	struct SFuncs<std::shared_ptr<T>, std::enable_if_t<std::is_base_of_v<Object, T> || std::is_same_v<std::string, T>>> {
		static inline void WriteTo(std::string& s, std::shared_ptr<T> const& in) noexcept {
			if (in) {
				SFuncs<T>::WriteTo(s, *in);
			}
			else {
				s.append("nil");
			}
		}
	};

	// 适配 std::weak_ptr<T>
	template<typename T>
	struct SFuncs<std::weak_ptr<T>, std::enable_if_t<std::is_base_of_v<Object, T> || std::is_same_v<std::string, T>>> {
		static inline void WriteTo(std::string& s, std::weak_ptr<T> const& in) noexcept {
			if (auto o = in.lock()) {
				SFuncs<T>::WriteTo(s, *o);
			}
			else {
				s.append("nil");
			}
		}
	};

	// utils

	inline size_t Calc2n(size_t const& n) noexcept {
		assert(n);
#ifdef _MSC_VER
		unsigned long r = 0;
#if defined(_WIN64) || defined(_M_X64)
		_BitScanReverse64(&r, n);
# else
		_BitScanReverse(&r, n);
# endif
		return (size_t)r;
#else
#if defined(__LP64__) || __WORDSIZE == 64
		return int(63 - __builtin_clzl(n));
# else
		return int(31 - __builtin_clz(n));
# endif
#endif
	}

	inline size_t Round2n(size_t const& n) noexcept {
		auto rtv = size_t(1) << Calc2n(n);
		if (rtv == n) return n;
		else return rtv << 1;
	}



	// std::cout 扩展

	//inline std::ostream& operator<<(std::ostream& os, const Object& o) {
	//	std::string s;
	//	o.ToString(s);
	//	os << s;
	//	return os;
	//}

	//template<typename T>
	//std::ostream& operator<<(std::ostream& os, std::shared_ptr<T> const& o) {
	//	if (!o) return os << "nil";
	//	return os << *o;
	//}

	//template<typename T>
	//std::ostream& operator<<(std::ostream& os, std::weak_ptr<T> const& o) {
	//	if (!o) return os << "nil";
	//	return os << *o;
	//}

	template<typename...Args>
	inline void Cout(Args const&...args) {
		std::string s;
		Append(s, args...);
		std::cout << s;
	}
	template<typename...Args>
	inline void CoutN(Args const&...args) {
		Cout(args...);
		std::cout << std::endl;
	}


	// make_shared, weak helpers

	template<typename T, typename ...Args>
	std::shared_ptr<T> Make(Args&&...args) {
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	template<typename T, typename ...Args>
	std::shared_ptr<T>& MakeTo(std::shared_ptr<T>& v, Args&&...args) {
		v = std::make_shared<T>(std::forward<Args>(args)...);
		return v;
	}

	template<typename ...Args>
	std::string_s MakeString(Args&&...args) {
		return std::make_shared<std::string>(std::forward<Args>(args)...);
	}

	template<typename T>
	std::weak_ptr<T> Weak(std::shared_ptr<T> const& v) {
		return std::weak_ptr<T>(v);
	}

	template<typename T, typename ...Args>
	std::shared_ptr<T> TryMake(Args&&...args) {
		try {
			return std::make_shared<T>(std::forward<Args>(args)...);
		}
		catch (...) {
			return std::shared_ptr<T>();
		}
	}

	template<typename T, typename ...Args>
	std::shared_ptr<T>& TryMakeTo(std::shared_ptr<T>& v, Args&&...args) {
		v = TryMake<T>(std::forward<Args>(args)...);
		return v;
	}

	template<typename T, typename U>
	std::shared_ptr<T> As(std::shared_ptr<U> const& v) {
		assert(std::dynamic_pointer_cast<T>(v));
		return *(std::shared_ptr<T>*)&v;
	}
	template<typename T, typename U>
	std::weak_ptr<T> AsWeak(std::shared_ptr<U> const& v) {
		return std::weak_ptr<T>(As<T>(v));
	}


	// helpers

	struct ScopeGuard {
		template<typename T>
		ScopeGuard(T&& f) : func(std::forward<T>(f)) {}
		~ScopeGuard() { Run(); }
		inline void RunAndCancel() noexcept { Run(); Cancel(); }
		inline void Run() noexcept { if (func) func(); }
		inline void Cancel() noexcept { func = nullptr; }
		template<typename T>
		inline void Set(T&& f) noexcept { func = std::forward<T>(f); }
	private:
		kapala::fixed_function<void()> func;
		ScopeGuard(ScopeGuard const&) = delete;
		ScopeGuard &operator=(ScopeGuard const&) = delete;
	};



	// guid

	struct Guid {
		union {
			struct {
				uint64_t part1;
				uint64_t part2;
			};
			struct {	// for ToString
				uint32_t  data1;
				unsigned short data2;
				unsigned short data3;
				unsigned char  data4[8];
			};
		};

		explicit Guid(bool const& gen = true) noexcept {
			if (gen) {
				Gen();
			}
			else {
				part1 = 0;
				part2 = 0;
			}
		}
		Guid(Guid const& o) noexcept = default;
		Guid& operator=(Guid const& o) noexcept = default;

		bool operator==(Guid const& o) const noexcept {
			return part1 == o.part1 && part2 == o.part2;
		}
		bool operator!=(Guid const& o) const noexcept {
			return part1 != o.part1 || part2 != o.part2;
		}

		void Gen() noexcept {
#ifdef _WIN32
			CoCreateGuid((GUID*)this);
#else
			uuid_generate((unsigned char*)this);
#endif
		}
		inline void Fill(char const* const& buf) noexcept {
			memcpy(this, buf, 16);
		}
		inline void Fill(uint8_t const* const& buf) noexcept {
			memcpy(this, buf, 16);
		}

		bool IsZero() noexcept {
			return part1 == 0 && part2 == 0;
		}
	};

	template<>
	struct SFuncs<Guid, void> {
		static inline void WriteTo(std::string& s, Guid const& in) noexcept {
			auto offset = s.size();
			s.resize(offset + 37);
			snprintf(s.data() + offset, 37,
				"%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
				in.data1, in.data2, in.data3,
				in.data4[0], in.data4[1],
				in.data4[2], in.data4[3],
				in.data4[4], in.data4[5],
				in.data4[6], in.data4[7]
			);
			s.resize(s.size() - 1);	// remove \0
		}
	};



}

namespace std {
	template<> 
	struct hash<xx::Guid> {
		std::size_t operator()(xx::Guid const& in) const noexcept {
			if constexpr (sizeof(std::size_t) == 8) {
				return in.part1 ^ in.part2;
			}
			else {
				return ((uint32_t*)&in)[0] ^ ((uint32_t*)&in)[1] ^ ((uint32_t*)&in)[2] ^ ((uint32_t*)&in)[3];
			}
		}
	};
}
