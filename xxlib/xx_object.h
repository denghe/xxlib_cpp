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




	/************************************************************************************/
	// time_point <--> .net DateTime.Now.ToUniversalTime().Ticks converts
	/************************************************************************************/

	/*

	// some example for output:

	#include <ctime>
	#include <iomanip>

	std::time_t t = std::time(nullptr);
	std::tm tm;
	localtime_s(&tm, &t);
	std::cout << std::put_time(&tm, "%Y-%m-%d %X") << std::endl;

	auto tp = xx::Epoch10mToTimePoint(15xxxxxxxxxxxxxxxxxxx);
	t = std::chrono::system_clock::to_time_t(tp);
	localtime_s(&tm, &t);
	std::cout << std::put_time(&tm, "%Y-%m-%d %X") << std::endl;

	// todo: epoch to string

	*/

	// 经历时间精度: 秒后 7 个 0( 这是 windows 下最高精度. android/ios 会低1个0的精度 )
	typedef std::chrono::duration<long long, std::ratio<1LL, 10000000LL>> duration_10m;

	// 时间点 转 epoch (精度为秒后 7 个 0)
	inline int64_t TimePointToEpoch10m(std::chrono::system_clock::time_point const& val) noexcept
	{
		return std::chrono::duration_cast<duration_10m>(val.time_since_epoch()).count();
	}

	//  epoch (精度为秒后 7 个 0) 转 时间点
	inline std::chrono::system_clock::time_point Epoch10mToTimePoint(int64_t const& val) noexcept
	{
		return std::chrono::system_clock::time_point(std::chrono::duration_cast<std::chrono::system_clock::duration>(duration_10m(val)));
	}


	// 得到当前时间点
	inline std::chrono::system_clock::time_point NowTimePoint() noexcept
	{
		return std::chrono::system_clock::now();
	}

	// 得到当前时间点的 epoch (精度为秒后 7 个 0)
	inline int64_t NowEpoch10m() noexcept
	{
		return TimePointToEpoch10m(NowTimePoint());
	}


	// epoch (精度为秒后 7 个 0) 转为 .Net DateTime Utc Ticks
	inline int64_t Epoch10mToUtcDateTimeTicks(int64_t const& val) noexcept
	{
		return val + 621355968000000000LL;
	}

	// .Net DateTime Utc Ticks 转为 epoch (精度为秒后 7 个 0)
	inline int64_t UtcDateTimeTicksToEpoch10m(int64_t const& val) noexcept
	{
		return val - 621355968000000000LL;
	}


	// 时间点 转 epoch (精度为秒)
	inline int32_t TimePointToEpoch(std::chrono::system_clock::time_point const& val) noexcept
	{
		return (int32_t)(val.time_since_epoch().count() / 10000000);
	}

	//  epoch (精度为秒) 转 时间点
	inline std::chrono::system_clock::time_point EpochToTimePoint(int32_t const& val) noexcept
	{
		return std::chrono::system_clock::time_point(std::chrono::system_clock::time_point::duration((int64_t)val * 10000000));
	}


	// 得到当前 system 时间点的 epoch (精度为 ms)
	inline int64_t NowSystemEpochMS() noexcept
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}
	// 得到当前 steady 时间点的 epoch (精度为 ms)
	inline int64_t NowSteadyEpochMS() noexcept
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
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
	std::weak_ptr<T> ToWeak(std::shared_ptr<T> const& v) {
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
	std::shared_ptr<T>& As(std::shared_ptr<U> const& v) {
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









	struct Stackless {
		using FuncType = std::function<int(int const& lineNumber)>;
		std::vector<std::pair<FuncType, int>> funcs;
		inline void Add(FuncType&& func) {
			if (!func) return;
			funcs.emplace_back(std::move(func), 0);
		}
		inline void RunAdd(FuncType&& func) {
			if (!func) return;
			int n = func(0);
			if (n == (int)0xFFFFFFFF) return;
			funcs.emplace_back(std::move(func), n);
		}
		size_t RunOnce() {
			if (funcs.size()) {
				for (auto&& i = funcs.size() - 1; i != (size_t)-1; --i) {
					auto&& func = funcs[i];
					func.second = func.first(func.second);
					if (!func.second) {
						if (i + 1 < funcs.size()) {
							funcs[i] = std::move(funcs[funcs.size() - 1]);
						}
						funcs.pop_back();
					}
				}
			}
			return funcs.size();
		}
	};

#define COR_BEGIN	switch (lineNumber) { case 0:
#define COR_YIELD	return __LINE__; case __LINE__:;
#define COR_END		} return 0;










	// 移动时是否可使用 memmove 的标志 基础适配模板
	template<typename T, typename ENABLED = void>
	struct IsTrivial {
		static const bool value = false;
	};

	template<typename T>
	constexpr bool IsTrivial_v = IsTrivial<T>::value;

	// 适配 std::is_trivial<T>::value
	template<typename T>
	struct IsTrivial<T, std::enable_if_t<std::is_trivial_v<T>>> {
		static const bool value = true;
	};








	// < 2G, 8 - 512 dataSize
	constexpr static const int32_t primes[] = { 7, 11, 13, 17, 19, 23, 31, 37, 43, 47, 53, 59, 67, 71, 73, 79, 83, 89, 97, 103, 107, 109, 113, 131, 139, 151, 157, 163, 167, 173, 179, 181, 191, 193, 199, 211, 223, 227, 229, 233, 239, 241, 251, 257, 263, 271, 277, 283, 293, 307, 311, 313, 317, 331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419, 421, 431, 433, 439, 443, 449, 457, 463, 467, 479, 487, 491, 499, 503, 509, 523, 541, 547, 557, 563, 571, 577, 587, 599, 607, 613, 619, 631, 647, 653, 661, 677, 683, 691, 701, 709, 719, 727, 733, 743, 751, 757, 761, 773, 787, 797, 811, 823, 829, 839, 853, 863, 877, 887, 911, 919, 929, 941, 947, 953, 967, 971, 983, 991, 997, 1013, 1039, 1051, 1069, 1087, 1103, 1117, 1129, 1151, 1163, 1181, 1193, 1213, 1231, 1237, 1259, 1279, 1291, 1307, 1327, 1373, 1381, 1399, 1423, 1439, 1453, 1471, 1487, 1499, 1511, 1531, 1549, 1567, 1583, 1597, 1613, 1627, 1637, 1663, 1669, 1693, 1709, 1723, 1741, 1759, 1789, 1801, 1823, 1831, 1847, 1871, 1879, 1901, 1913, 1933, 1951, 1979, 1999, 2011, 2029, 2069, 2111, 2143, 2161, 2207, 2239, 2269, 2297, 2333, 2357, 2399, 2423, 2459, 2477, 2521, 2557, 2591, 2621, 2647, 2687, 2719, 2749, 2777, 2803, 2843, 2879, 2909, 2939, 2971, 3001, 3037, 3067, 3089, 3121, 3167, 3191, 3229, 3259, 3271, 3323, 3359, 3391, 3413, 3449, 3469, 3517, 3547, 3583, 3613, 3643, 3677, 3709, 3739, 3769, 3803, 3833, 3863, 3889, 3931, 3967, 3989, 4027, 4057, 4159, 4219, 4283, 4349, 4409, 4463, 4523, 4603, 4663, 4733, 4799, 4861, 4919, 4987, 5051, 5119, 5179, 5237, 5309, 5351, 5437, 5503, 5563, 5623, 5693, 5749, 5821, 5881, 5939, 6011, 6079, 6143, 6203, 6271, 6329, 6397, 6451, 6521, 6581,
		6653, 6719, 6781, 6841, 6911, 6971, 7039, 7103, 7159, 7229, 7283, 7351, 7417, 7487, 7549, 7607, 7673, 7741, 7793, 7867, 7933, 7993, 8059, 8123, 8317, 8447, 8573, 8699, 8831, 8951, 9067, 9209, 9343, 9467, 9587, 9721, 9851, 9973, 10111, 10223, 10357, 10487, 10613, 10739, 10867, 11003, 11131, 11261, 11383, 11519, 11633, 11743, 11903, 12011, 12157, 12281, 12413, 12541, 12671, 12799, 12923, 13049, 13183, 13309, 13421, 13567, 13693, 13807, 13933, 14071, 14207, 14327, 14461, 14591, 14717, 14843, 14969, 15101, 15227, 15359, 15473, 15607, 15739, 15859, 15991, 16127, 16253, 16633, 16889, 17137, 17401, 17659, 17911, 18169, 18427, 18679, 18919, 19183, 19447, 19709, 19963, 20219, 20479, 20731, 20983, 21247, 21503, 21757, 22013, 22271, 22511, 22783, 23039, 23293, 23549, 23801, 24061, 24317, 24571, 24821, 25087, 25343, 25589, 25849, 26111, 26357, 26597, 26879, 27127, 27367, 27647, 27901, 28151, 28411, 28669, 28927, 29179, 29437, 29683, 29947, 30203, 30449, 30713, 30971, 31231, 31481, 31741, 31991, 32251, 32507, 33247, 33791, 34303, 34807, 35327, 35839, 36343, 36857, 37369, 37879, 38393, 38903, 39419, 39929, 40433, 40949, 41467, 41983, 42491, 43003, 43517, 44029, 44543, 45053, 45557, 46073, 46591, 47093, 47609, 48121, 48623, 49139, 49663, 50159, 50683, 51199, 51691, 52223, 52733, 53239, 53759, 54269, 54779, 55291, 55807, 56311, 56827, 57331, 57853, 58367, 58831, 59387, 59887, 60413, 60923, 61417, 61949, 62459, 62971, 63487, 63997, 64499, 65011, 66553, 67579, 68597, 69623, 70639, 71671, 72701, 73727, 74747, 75773, 76781, 77813, 78839, 79867, 80863, 81919, 82939, 83939, 84991, 86011, 87037, 88037, 89087, 90107, 91129,
		92153, 93179, 94207, 95231, 96233, 97259, 98299, 99317, 100343, 101363, 102397, 103423, 104417, 105467, 106487, 107509, 108541, 109567, 110587, 111611, 112621, 113657, 114679, 115693, 116731, 117757, 118757, 119797, 120829, 121853, 122869, 123887, 124919, 125941, 126967, 127997, 129023, 130043, 133117, 135151, 137209, 139241, 141311, 143357, 145399, 147451, 149503, 151549, 153589, 155627, 157679, 159739, 161783, 163819, 165887, 167917, 169957, 172031, 174079, 176123, 178169, 180221, 182261, 184309, 186343, 188407, 190409, 192499, 194543, 196597, 198647, 200699, 202751, 204797, 206827, 208891, 210943, 212987, 214993, 217081, 219133, 221173, 223229, 225263, 227303, 229373, 231419, 233437, 235519, 237563, 239611, 241663, 243709, 245759, 247799, 249853, 251903, 253951, 255989, 258031, 260089, 266239, 270329, 274423, 278503, 282617, 286711, 290803, 294911, 298999, 303097, 307189, 311293, 315389, 319483, 323581, 327673, 331769, 335857, 339959, 344053, 348149, 352249, 356351, 360439, 364543, 368633, 372733, 376823, 380917, 385013, 389117, 393209, 397303, 401407, 405499, 409597, 413689, 417773, 421847, 425977, 430061, 434167, 438271, 442367, 446461, 450557, 454637, 458747, 462841, 466919, 471007, 475109, 479231, 483323, 487423, 491503, 495613, 499711, 503803, 507901, 511997, 516091, 520151, 532453, 540629, 548861, 557041, 565247, 573437, 581617, 589811, 598007, 606181, 614387, 622577, 630737, 638971, 647161, 655357, 663547,
		671743, 679933, 688111, 696317, 704507, 712697, 720887, 729073, 737279, 745471, 753659, 761833, 770047, 778237, 786431, 794593, 802811, 810989, 819187, 827389, 835559, 843763, 851957, 860143, 868349, 876529, 884717, 892919, 901111, 909301, 917503, 925679, 933883, 942079, 950269, 958459, 966653, 974837, 982981, 991229, 999389, 1007609, 1015769, 1023991, 1032191, 1040381, 1064957, 1081337, 1097717, 1114111, 1130471, 1146877, 1163263, 1179641, 1196029, 1212401, 1228789, 1245169, 1261567, 1277911, 1294309, 1310719, 1327099, 1343479, 1359871, 1376237, 1392631, 1409017, 1425371, 1441771, 1458169, 1474559, 1490941, 1507321, 1523707, 1540087, 1556473, 1572853, 1589239, 1605631, 1622009, 1638353, 1654739, 1671161, 1687549, 1703903, 1720307, 1736701, 1753069, 1769441, 1785853, 1802239, 1818617, 1835003, 1851391, 1867771, 1884133, 1900543, 1916921, 1933301, 1949657, 1966079, 1982447, 1998839, 2015213, 2031611, 2047993, 2064379, 2080763, 2129903, 2162681, 2195443, 2228221, 2260967, 2293757, 2326517, 2359267, 2392057, 2424827, 2457569, 2490337, 2523133, 2555897, 2588671, 2621431, 2654161, 2686973, 2719741, 2752499, 2785273, 2818043, 2850811, 2883577, 2916343, 2949119, 2981887, 3014653, 3047423, 3080167, 3112943, 3145721, 3178489, 3211213, 3244013, 3276799, 3309563, 3342331, 3375083, 3407857, 3440627, 3473399, 3506171, 3538933, 3571699, 3604451, 3637223, 3670013, 3702757, 3735547, 3768311, 3801073, 3833833, 3866623, 3899383, 3932153, 3964913, 3997673, 4030463, 4063217, 4095991, 4128767, 4161527, 4259837, 4325359, 4390909,
		4456433, 4521977, 4587503, 4653041, 4718579, 4784107, 4849651, 4915171, 4980727, 5046259, 5111791, 5177339, 5242877, 5308379, 5373931, 5439479, 5505023, 5570533, 5636077, 5701627, 5767129, 5832679, 5898209, 5963773, 6029299, 6094807, 6160381, 6225917, 6291449, 6356983, 6422519, 6488023, 6553577, 6619111, 6684659, 6750203, 6815741, 6881269, 6946813, 7012337, 7077883, 7143421, 7208951, 7274489, 7340009, 7405547, 7471099, 7536637, 7602151, 7667711, 7733233, 7798783, 7864301, 7929833, 7995391, 8060891, 8126453, 8191991, 8257531, 8323069, 8519647, 8650727, 8781797, 8912887, 9043967, 9175037, 9306097, 9437179, 9568219, 9699323, 9830393, 9961463, 10092539, 10223593, 10354667, 10485751, 10616831, 10747903, 10878961, 11010037, 11141113, 11272181, 11403247, 11534329, 11665403, 11796469, 11927551, 12058621, 12189677, 12320753, 12451807, 12582893, 12713959, 12845033, 12976121, 13107197, 13238263, 13369333, 13500373, 13631477, 13762549, 13893613, 14024671, 14155763, 14286809, 14417881, 14548979, 14680063, 14811133, 14942197, 15073277, 15204349, 15335407, 15466463, 15597559, 15728611, 15859687, 15990781, 16121849, 16252919, 16383977, 16515067, 16646099, 17039339, 17301463, 17563633, 17825791, 18087899, 18350063, 18612211, 18874367, 19136503, 19398647, 19660799, 19922923, 20185051, 20447191, 20709347, 20971507, 21233651, 21495797, 21757951, 22020091, 22282199, 22544351, 22806521, 23068667, 23330773, 23592937, 23855101, 24117217, 24379391, 24641479, 24903667, 25165813, 25427957, 25690097, 25952243, 26214379, 26476543, 26738687,
		27000817, 27262931, 27525109, 27787213, 28049407, 28311541, 28573673, 28835819, 29097977, 29360087, 29622269, 29884411, 30146531, 30408701, 30670847, 30932987, 31195117, 31457269, 31719409, 31981567, 32243707, 32505829, 32767997, 33030121, 33292283, 34078699, 34602991, 35127263, 35651579, 36175871, 36700159, 37224437, 37748717, 38273023, 38797303, 39321599, 39845887, 40370173, 40894457, 41418739, 41943023, 42467317, 42991609, 43515881, 44040187, 44564461, 45088739, 45613039, 46137319, 46661627, 47185907, 47710207, 48234451, 48758783, 49283063, 49807327, 50331599, 50855899, 51380179, 51904511, 52428767, 52953077, 53477357, 54001663, 54525917, 55050217, 55574507, 56098813, 56623093, 57147379, 57671671, 58195939, 58720253, 59244539, 59768831, 60293119, 60817397, 61341659, 61865971, 62390261, 62914549, 63438839, 63963131, 64487417, 65011703, 65535989, 66060277, 66584561, 68157433, 69205987, 70254563, 71303153, 72351733, 73400311, 74448877, 75497467, 76546039, 77594599, 78643199, 79691761, 80740339, 81788923, 82837501, 83886053, 84934621, 85983217, 87031759, 88080359, 89128939, 90177533, 91226101, 92274671, 93323249, 94371833, 95420401, 96468979, 97517543, 98566121, 99614689, 100663291, 101711839, 102760387, 103809011, 104857589, 105906167, 106954747, 108003323, 109051903, 110100409, 111148963, 112197629, 113246183, 114294721, 115343341, 116391917, 117440509, 118489081, 119537653, 120586231, 121634801, 122683391,
		123731963, 124780531, 125829103, 126877693, 127926263, 128974841, 130023407, 131071987, 132120557, 133169137, 136314869, 138412031, 140509183, 142606333, 144703477, 146800637, 148897789, 150994939, 153092087, 155189239, 157286383, 159383467, 161480689, 163577833, 165674981, 167772107, 169869311, 171966439, 174063611, 176160739, 178257917, 180355069, 182452199, 184549373, 186646517, 188743679, 190840817, 192937933, 195035129, 197132267, 199229419, 201326557, 203423729, 205520881, 207618031, 209715199, 211812331, 213909503, 216006599, 218103799, 220200947, 222298093, 224395253, 226492393, 228589561, 230686697, 232783871, 234881011, 236978171, 239075327, 241172479, 243269627, 245366777, 247463933, 249561083, 251658227, 253755391, 255852511, 257949691, 260046847, 262143977, 264241147, 266338297, 272629759, 276824033, 281018263, 285212659, 289406951, 293601263, 297795541, 301989881, 306184189, 310378493, 314572799, 318767093, 322961399, 327155693, 331349989, 335544301, 339738617, 343932923, 348127231, 352321531, 356515813, 360710137, 364904447, 369098707, 373293049, 377487343, 381681661, 385875929, 390070267, 394264567, 398458859, 402653171, 406847479, 411041767, 415236083, 419430383, 423624673, 427819001, 432013291, 436207613, 440401889, 444596209, 448790519, 452984827, 457179131, 461373431, 465567743, 469762043, 473956339, 478150643, 482344957, 486539257, 490733567, 494927857, 499122163, 503316469, 507510719, 511705069, 515899379,
		520093667, 524287997, 528482263, 532676593, 545259493, 553648103, 562036693, 570425299, 578813951, 587202547, 595591153, 603979769, 612368377, 620756953, 629145593, 637534199, 645922787, 654311423, 662700019, 671088637, 679477243, 687865847, 696254441, 704643067, 713031659, 721420261, 729808889, 738197503, 746586109, 754974691, 763363327, 771751927, 780140521, 788529121, 796917757, 805306357, 813694951, 822083551, 830472127, 838860791, 847249399, 855637957, 864026567, 872415211, 880803809, 889192441, 897581029, 905969653, 914358253, 922746863, 931135487, 939524087, 947912701, 956301169, 964689917, 973078511, 981467119, 989855707, 998244341, 1006632947, 1015021549, 1023410159, 1031798783, 1040187377, 1048575959, 1056964501, 1065353209, 1090519013, 1107296251, 1124073463, 1140850681, 1157627879, 1174405103, 1191182309, 1207959503, 1224736717, 1241513981, 1258291187, 1275068407, 1291845593, 1308622837, 1325400059, 1342177237, 1358954453, 1375731701, 1392508927, 1409286139, 1426063351, 1442840569, 1459617779, 1476394991, 1493172223, 1509949421, 1526726647, 1543503851, 1560281087, 1577058271, 1593835489, 1610612711, 1627389901, 1644167159, 1660944367, 1677721597, 1694498801, 1711276019, 1728053237, 1744830457, 1761607657, 1778384887, 1795162111, 1811939317, 1828716523, 1845493753, 1862270957, 1879048183, 1895825387, 1912602623, 1929379813, 1946157053, 1962934271, 1979711483, 1996488677, 2013265907, 2030043131, 2046820351, 2063597567, 2080374781, 2097151967, 2113929203, 2130706381 };

	//  < 2G 2^N
	constexpr static const int32_t primes2n[] =
	{
		1, 2, 3, 7, 13, 31, 61, 127, 251, 509, 1021, 2039, 4093, 8191, 16381, 32749, 65521, 131071, 262139, 524287,
		1048573, 2097143, 4194301, 8388593, 16777213, 33554393, 67108859, 134217689, 268435399, 536870909, 1073741789
	};

	inline bool IsPrime(size_t const& candidate) noexcept
	{
		if ((candidate & 1) != 0) {
			size_t limit = size_t(std::sqrt(candidate));
			for (size_t divisor = 3; divisor <= limit; divisor += 2) {
				if ((candidate % divisor) == 0) return false;
			}
			return true;
		}
		return (candidate == 2);
	}

	inline int32_t GetPrime(int32_t const& capacity, int32_t const& dataSize) noexcept {
		auto memUsage = Round2n(capacity * dataSize);
		auto maxCapacity = memUsage / dataSize;
		if (maxCapacity == capacity) {
			return primes2n[Calc2n(capacity)];
		}
		if (dataSize >= 8 && dataSize <= 512) {                     // 数据长在 查表 范围内的
			return *std::upper_bound(std::begin(primes), std::end(primes), (int32_t)maxCapacity);
		}
		for (size_t i = maxCapacity + 1; i <= 0x7fffffff; i += 2) { // maxCapacity 是双数. +1 即为单数
			if (IsPrime(i)) return (int32_t)i;
		}
		assert(false);
		return -1;
	}

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
