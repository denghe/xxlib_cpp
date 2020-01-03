#pragma once
#include "xx_list.h"

namespace xx {
	struct BBuffer;
	using BBuffer_s = std::shared_ptr<BBuffer>;
	using BBuffer_w = std::weak_ptr<BBuffer>;

	struct BBuffer : List<uint8_t> {
		using BaseType = List<uint8_t>;

		size_t offset = 0;													// 读指针偏移量
		size_t offsetRoot = 0;												// offset值写入修正
		size_t readLengthLimit = 0;											// 主用于传递给容器类进行长度合法校验

		// todo: 这些容器改为指针, XxxxxRoot 函数中检测并创建
		std::unordered_map<void*, size_t> ptrs;
		std::unordered_map<size_t, std::shared_ptr<Object>> objIdxs;
		std::unordered_map<size_t, std::shared_ptr<std::string>> strIdxs;

		BBuffer() : BaseType() {}
		BBuffer(BBuffer&& o) noexcept
			: BaseType(std::move(o))
			, offset(o.offset) {
			o.offset = 0;
		}
		inline BBuffer& operator=(BBuffer&& o) noexcept {
			this->BaseType::operator=(std::move(o));
			std::swap(offset, o.offset);
			// ptrs, objIdxs, strIdxs 因为是临时数据, 不需要处理
			return *this;
		}
		BBuffer(BBuffer const&) = delete;
		BBuffer& operator=(BBuffer const&) = delete;

		// unsafe: direct change field value( for Read )
		inline void Reset(uint8_t* const& buf = nullptr, size_t const& len = 0, size_t const& cap = 0, size_t const& offset = 0) noexcept {
			this->buf = buf;
			this->len = len;
			this->cap = cap;
			this->offset = offset;
		}


		// todo: creators, Register, CreateByTypeId 变非静态, creators 移到生成物, ReadRoot WriteRoot 前可切换 creators 以便同时支持多份 PKG 生成物

		typedef std::shared_ptr<Object>(*Creator)();
		inline static std::array<Creator, 1 << (sizeof(uint16_t) * 8)> creators;

		template<typename T, typename ENABLED = std::enable_if_t<std::is_base_of_v<Object, T>>>
		inline static void Register(uint16_t const& typeId) noexcept {
			creators[typeId] = []()->std::shared_ptr<Object> { return xx::TryMake<T>(); };
		}

		inline static std::shared_ptr<Object> CreateByTypeId(uint16_t typeId) {
			return creators[typeId] ? creators[typeId]() : std::shared_ptr<Object>();
		}

		template<typename T, typename ENABLED = std::enable_if_t<std::is_pod_v<T>>>
		void WriteFixed(T const& v) {
			Reserve(len + sizeof(T));
			memcpy(buf + len, &v, sizeof(T));
			len += sizeof(T);
		}

		template<typename T, typename ENABLED = std::enable_if_t<std::is_pod_v<T>>>
		int ReadFixed(T& v) {
			if (offset + sizeof(T) > len) return -1;
			memcpy(&v, buf + offset, sizeof(T));
			offset += sizeof(T);
			return 0;
		}

		template<typename ...TS>
		void Write(TS const& ...vs) noexcept;

		template<typename T, typename ...TS>
		int ReadCore(T& v, TS&...vs) noexcept;

		template<typename T>
		int ReadCore(T& v) noexcept;

		template<typename ...TS>
		int Read(TS&...vs) noexcept;

		template<typename T>
		void WriteRoot(std::shared_ptr<T> const& v) noexcept {
			offsetRoot = len;
			assert(ptrs.empty());
			Write(v);
			ptrs.clear();
		}

		template<typename T>
		int ReadRoot(std::shared_ptr<T>& v) noexcept {
			offsetRoot = offset;
			assert(objIdxs.empty());
			assert(strIdxs.empty());
			int r = Read(v);
			objIdxs.clear();
			strIdxs.clear();
			return r;
		}

		template<typename T>
		void WritePtr(std::shared_ptr<T> const& v) noexcept {
			static_assert(std::is_base_of_v<Object, T> || std::is_same_v<std::string, T>, "not support type??");
			uint16_t typeId = 0;
			if (!v) {
				Write(typeId);
				return;
			}
			if constexpr (std::is_base_of_v<Object, T>) {
				typeId = v->GetTypeId();
				assert(typeId);					// forget Register TypeId ? 
			}
			else typeId = 1;					// std::string

			Write(typeId);

			auto iter = ptrs.find((void*)&*v);
			size_t offs;
			if (iter == ptrs.end()) {
				offs = len - offsetRoot;
				ptrs[(void*)&*v] = offs;
			}
			else {
				offs = iter->second;
			}
			Write(offs);
			if (iter == ptrs.end()) {
				if constexpr (std::is_same_v<std::string, T>) {
					Write(*v);
				}
				else {
					v->ToBBuffer(*this);
				}
			}
		}

		template<typename T>
		int ReadPtr(std::shared_ptr<T>& v) noexcept {
			static_assert(std::is_base_of_v<Object, T> || std::is_same_v<std::string, T>, "not support type??");
			v.reset();
			uint16_t typeId;
			if (auto r = Read(typeId)) return r;
			if (typeId == 0) return 0;
			if constexpr (std::is_same_v<std::string, T>) {
				if (typeId != 1) return -1;
			}
			else if constexpr (std::is_base_of_v<Object, T>) {
				if (typeId < 2) return -2;
			}
			if (typeId > 2 && !creators[typeId]) return -3;		// forget Register?

			auto offs = offset - offsetRoot;
			size_t ptrOffset;
			if (auto r = Read(ptrOffset)) return r;
			if (ptrOffset == offs) {
				if constexpr (std::is_same_v<std::string, T>) {
					v = xx::TryMake<std::string>();
					strIdxs[ptrOffset] = v;
					if (auto r = Read(*v)) return r;
				}
				else {
					std::shared_ptr<Object> o;
					if (typeId == 2) {
						o = xx::TryMake<BBuffer>();
					}
					else {
						o = CreateByTypeId(typeId);
						if (!o) return -3;
					}
					v = std::dynamic_pointer_cast<T>(o);
					if (!v) return -4;
					objIdxs[ptrOffset] = o;
					if (auto r = o->FromBBuffer(*this)) return r;
				}
			}
			else {
				if constexpr (std::is_same_v<std::string, T>) {
					auto iter = strIdxs.find(ptrOffset);
					if (iter == strIdxs.end()) return -5;
					v = iter->second;
				}
				else {
					auto iter = objIdxs.find(ptrOffset);
					if (iter == objIdxs.end()) return -6;
					if (iter->second->GetTypeId() != typeId) return -7;
					v = std::dynamic_pointer_cast<T>(iter->second);
					if (!v) return -8;
				}
			}
			return 0;
		}

		template<typename T, bool needReserve = true>
		inline void WriteVarIntger(T const& v) {
			using UT = std::make_unsigned_t<T>;
			UT u(v);
			if constexpr (std::is_signed_v<T>) {
				u = ZigZagEncode(v);
			}
			if constexpr (needReserve) {
				Reserve(len + sizeof(T) + 1);
			}
			while (u >= 1 << 7) {
				buf[len++] = uint8_t((u & 0x7fu) | 0x80u);
				u = UT(u >> 7);
			};
			buf[len++] = uint8_t(u);
		}

		template<typename T>
		inline int ReadVarInteger(T& v) {
			using UT = std::make_unsigned_t<T>;
			UT u(0);
			for (size_t shift = 0; shift < sizeof(T) * 8; shift += 7) {
				if (offset == len) return -9;
				auto b = (UT)buf[offset++];
				u |= UT((b & 0x7Fu) << shift);
				if ((b & 0x80) == 0) {
					if constexpr (std::is_signed_v<T>) {
						v = ZigZagDecode(u);
					}
					else {
						v = u;
					}
					return 0;
				}
			}
			return -10;
		}


		inline virtual uint16_t GetTypeId() const noexcept override {
			return 2;
		}

		inline virtual void ToBBuffer(BBuffer& bb) const noexcept override {
			assert(this != &bb);
			bb.Write(len);
			bb.AddRange(buf, len);
		}

		inline virtual int FromBBuffer(BBuffer& bb) noexcept override {
			assert(this != &bb);
			size_t len = 0;
			if (auto r = bb.Read(len)) return r;
			if (bb.offset + len > bb.len) return -11;
			Clear();
			AddRange(bb.buf + bb.offset, len);
			bb.offset += len;
			return 0;
		}

		inline virtual void ToString(std::string& s) const noexcept override {
			Append(s, "{ \"len\":", len, ", \"cap\":", cap, ", \"offset\":", offset, ", \"buf\":[ ");
			for (size_t i = 0; i < len; i++) {
				Append(s, (int)buf[i], ", ");
			}
			if (len) s.resize(s.size() - 2);
			s += " ] }";
		}
	};

	template<typename T, size_t reservedHeaderLen>
	void List<T, reservedHeaderLen>::ToBBuffer(BBuffer& bb) const noexcept {
		bb.Reserve(bb.len + 5 + len * sizeof(T));
		bb.Write(len);
		if (!len) return;
		if constexpr (sizeof(T) == 1 || std::is_same_v<float, T>) {
			::memcpy(bb.buf + bb.len, buf, len * sizeof(T));
			bb.len += len * sizeof(T);
		}
		else {
			for (size_t i = 0; i < len; ++i) {
				bb.Write(buf[i]);
			}
		}
	}
	template<typename T, size_t reservedHeaderLen>
	int List<T, reservedHeaderLen>::FromBBuffer(BBuffer& bb) noexcept {
		size_t len = 0;
		if (auto rtv = bb.Read(len)) return rtv;
		if (bb.readLengthLimit != 0 && len > bb.readLengthLimit) return -1;
		if (bb.offset + len > bb.len) return -2;
		Resize(len);
		if (len == 0) return 0;
		if constexpr (sizeof(T) == 1 || std::is_same_v<float, T>) {
			::memcpy(buf, bb.buf + bb.offset, len * sizeof(T));
			bb.offset += len * sizeof(T);
		}
		else {
			for (size_t i = 0; i < len; ++i) {
				if (int r = bb.Read(buf[i])) return r;
			}
		}
		return 0;
	}

	// 适配 1 字节长度的 数值 或 float( 这些类型直接 memcpy )
	template<typename T>
	struct BFuncs<T, std::enable_if_t< (std::is_arithmetic_v<T> && sizeof(T) == 1) || (std::is_floating_point_v<T> && sizeof(T) == 4) >> {
		static inline void WriteTo(BBuffer& bb, T const &in) noexcept {
			bb.Reserve(bb.len + sizeof(T));
			memcpy(bb.buf + bb.len, &in, sizeof(T));
			bb.len += sizeof(T);
		}
		static inline int ReadFrom(BBuffer& bb, T &out) noexcept {
			if (bb.offset + sizeof(T) > bb.len) return -12;
			memcpy(&out, bb.buf + bb.offset, sizeof(T));
			bb.offset += sizeof(T);
			return 0;
		}
	};

	// 适配 2+ 字节整数( 变长读写 )
	template<typename T>
	struct BFuncs<T, std::enable_if_t<std::is_integral_v<T> && sizeof(T) >= 2>> {
		static inline void WriteTo(BBuffer& bb, T const &in) noexcept {
			bb.WriteVarIntger(in);
		}
		static inline int ReadFrom(BBuffer& bb, T &out) noexcept {
			return bb.ReadVarInteger(out);
		}
	};

	// 适配 enum( 根据原始数据类型调上面的适配 )
	template<typename T>
	struct BFuncs<T, std::enable_if_t<std::is_enum_v<T>>> {
		typedef std::underlying_type_t<T> UT;
		static inline void WriteTo(BBuffer& bb, T const &in) noexcept {
			bb.Write((UT const&)in);
		}
		static inline int ReadFrom(BBuffer& bb, T &out) noexcept {
			return bb.Read((UT&)out);
		}
	};

	// 适配 double
	template<>
	struct BFuncs<double, void> {
		static inline void WriteTo(BBuffer& bb, double const &in) noexcept {
			bb.Reserve(bb.len + sizeof(double) + 1);
			if (in == 0) {
				bb.buf[bb.len++] = 0;
			}
			else if (std::isnan(in)) {
				bb.buf[bb.len++] = 1;
			}
			else if (in == -std::numeric_limits<double>::infinity()) {	// negative infinity
				bb.buf[bb.len++] = 2;
			}
			else if (in == std::numeric_limits<double>::infinity()) {	// positive infinity
				bb.buf[bb.len++] = 3;
			}
			else {
				auto i = (int32_t)in;
				if (in == (double)i) {
					bb.buf[bb.len++] = 4;
					bb.WriteVarIntger<int32_t, false>(i);
				}
				else {
					bb.buf[bb.len] = 5;
					memcpy(bb.buf + bb.len + 1, &in, sizeof(double));
					bb.len += sizeof(double) + 1;
				}
			}
		}
		static inline int ReadFrom(BBuffer& bb, double &out) noexcept {
			if (bb.offset >= bb.len) return -13;	// 确保还有 1 字节可读
			switch (bb.buf[bb.offset++]) {			// 跳过 1 字节
			case 0:
				out = 0;
				return 0;
			case 1:
				out = std::numeric_limits<double>::quiet_NaN();
				return 0;
			case 2:
				out = -std::numeric_limits<double>::infinity();
				return 0;
			case 3:
				out = std::numeric_limits<double>::infinity();
				return 0;
			case 4: {
				int32_t i = 0;
				if (auto rtv = BFuncs<int32_t>::ReadFrom(bb, i)) return rtv;
				out = i;
				return 0;
			}
			case 5: {
				if (bb.len < bb.offset + sizeof(double)) return -14;
				memcpy(&out, bb.buf + bb.offset, sizeof(double));
				bb.offset += sizeof(double);
				return 0;
			}
			default:
				return -15;							// failed
			}
		}
	};


	// 适配 literal char[len] string  ( 写入 32b长度 + 内容. 不写入末尾 0 )
	template<size_t len>
	struct BFuncs<char[len], void> {
		static inline void WriteTo(BBuffer& bb, char const(&in)[len]) noexcept {
			bb.Write((size_t)(len - 1));
			bb.AddRange((uint8_t*)in, len - 1);
		}
		static inline int ReadFrom(BBuffer& bb, char (&out)[len]) noexcept {
			size_t readLen = 0;
			if (auto r = bb.Read(readLen)) return r;
			if (bb.readLengthLimit && bb.readLengthLimit < readLen) return -18;
			if (bb.offset + readLen > bb.len) return -19;
			if (readLen >= len) return -20;
			memcpy(out, bb.buf + bb.offset, readLen);
			out[readLen] = 0;
			bb.offset += readLen;
			return 0;
		}
	};

	// 适配 std::string ( 写入 变长长度 + 内容 )
	template<>
	struct BFuncs<std::string, void> {
		static inline void WriteTo(BBuffer& bb, std::string const& in) noexcept {
			bb.Write(in.size());
			bb.AddRange((uint8_t*)in.data(), in.size());
		}
		static inline int ReadFrom(BBuffer& bb, std::string& out) noexcept {
			size_t len = 0;
			if (auto r = bb.Read(len)) return r;
			if (bb.readLengthLimit && bb.readLengthLimit < len) return -16;
			if (bb.offset + len > bb.len) return -17;
			out.assign((char*)bb.buf + bb.offset, len);
			bb.offset += len;
			return 0;
		}
	};

	// 适配 std::shared_ptr<T>
	template<typename T>
	struct BFuncs<std::shared_ptr<T>, std::enable_if_t<std::is_base_of_v<Object, T> || std::is_same_v<std::string, T>>> {
		static inline void WriteTo(BBuffer& bb, std::shared_ptr<T> const& in) noexcept {
			bb.WritePtr(in);
		}
		static inline int ReadFrom(BBuffer& bb, std::shared_ptr<T>& out) noexcept {
			return bb.ReadPtr(out);
		}
	};

	// 适配 std::weak_ptr<T>
	template<typename T>
	struct BFuncs<std::weak_ptr<T>, std::enable_if_t<std::is_base_of_v<Object, T> || std::is_same_v<std::string, T>>> {
		static inline void WriteTo(BBuffer& bb, std::weak_ptr<T> const& in) noexcept {
			if (auto ptr = in.lock()) {
				bb.WritePtr(ptr);
			}
			else {
				bb.Write((uint16_t)0);
			}
		}
		static inline int ReadFrom(BBuffer& bb, std::weak_ptr<T>& out) noexcept {
			std::shared_ptr<T> ptr;
			if (int r = bb.ReadPtr(ptr)) return r;
			out = ptr;
			return 0;
		}
	};

	// 适配 Guid
	template<>
	struct BFuncs<Guid, void> {
		static inline void WriteTo(BBuffer& bb, Guid const& in) noexcept {
			bb.AddRange((uint8_t*)&in, sizeof(Guid));
		}
		static inline int ReadFrom(BBuffer& bb, Guid& out) noexcept {
			if (bb.offset + sizeof(Guid) > bb.len) return -1;
			memcpy(&out, bb.buf + bb.offset, sizeof(Guid));
			bb.offset += sizeof(Guid);
			return 0;
		}
	};

	// 适配 std::optional<T>
	template<typename T>
	struct BFuncs<std::optional<T>, void> {
		static inline void WriteTo(BBuffer& bb, std::optional<T> const& in) noexcept {
			if (in.has_value()) {
				bb.Write((uint8_t)1, in.value());
			}
			else {
				bb.Write((uint8_t)0);
			}
		}
		static inline int ReadFrom(BBuffer& bb, std::optional<T>& out) noexcept {
			uint8_t hasValue = 0;
			if (int r = bb.Read(hasValue)) return r;
			if (!hasValue) return 0;
			if (!out.has_value()) {
				out.emplace();
			}
			return bb.Read(out.value());
		}
	};

	// 适配 xx::List<T>
	template<typename T>
	struct BFuncs<List<T>, void> {
		static inline void WriteTo(BBuffer& bb, List<T> const& in) noexcept {
			in.ToBBuffer(bb);
		}
		static inline int ReadFrom(BBuffer& bb, List<T>& out) noexcept {
			return out.FromBBuffer(bb);
		}
	};

	// 适配 std::vector<T>
	template<typename T>
	struct BFuncs<std::vector<T>, void> {
		static inline void WriteTo(BBuffer& bb, std::vector<T> const& in) noexcept {
			auto buf = in.data();
			auto len = in.size();
			bb.Reserve(bb.len + 5 + len * sizeof(T));
			bb.Write(len);
			if (!len) return;
			if constexpr (sizeof(T) == 1 || std::is_same_v<float, T>) {
				::memcpy(bb.buf + bb.len, buf, len * sizeof(T));
				bb.len += len * sizeof(T);
			}
			else {
				for (size_t i = 0; i < len; ++i) {
					bb.Write(buf[i]);
				}
			}
		}
		static inline int ReadFrom(BBuffer& bb, std::vector<T>& out) noexcept {
			size_t len = 0;
			if (auto rtv = bb.Read(len)) return rtv;
			if (bb.readLengthLimit != 0 && len > bb.readLengthLimit) return -1;
			if (bb.offset + len > bb.len) return -2;
			out.resize(len);
			auto buf = out.data();
			if (len == 0) return 0;
			if constexpr (sizeof(T) == 1 || std::is_same_v<float, T>) {
				::memcpy(buf, bb.buf + bb.offset, len * sizeof(T));
				bb.offset += len * sizeof(T);
			}
			else {
				for (size_t i = 0; i < len; ++i) {
					if (int r = bb.Read(buf[i])) return r;
				}
			}
			return 0;
		}
	};


	// 适配 xx::BBuffer
	template<>
	struct BFuncs<BBuffer, void> {
		static inline void WriteTo(BBuffer& bb, BBuffer const& in) noexcept {
			in.ToBBuffer(bb);
		}
		static inline int ReadFrom(BBuffer& bb, BBuffer& out) noexcept {
			return out.FromBBuffer(bb);
		}
	};

	template<typename ...TS>
	void BBuffer::Write(TS const& ...vs) noexcept {
		std::initializer_list<int> n{ (BFuncs<TS>::WriteTo(*this, vs), 0)... };
		(void)n;
	}

	template<typename T, typename ...TS>
	int BBuffer::ReadCore(T& v, TS&...vs) noexcept {
		if (auto r = BFuncs<T>::ReadFrom(*this, v)) return r;
		return ReadCore(vs...);
	}

	template<typename T>
	int BBuffer::ReadCore(T& v) noexcept {
		return BFuncs<T>::ReadFrom(*this, v);
	}

	template<typename ...TS>
	int BBuffer::Read(TS&...vs) noexcept {
		return ReadCore(vs...);
	}
}
