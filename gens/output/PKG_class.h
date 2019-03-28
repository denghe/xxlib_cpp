#pragma once
namespace PKG {
	struct PkgGenMd5 {
		inline static const std::string value = "47f1f7e598b70be608364d963a2c7534";
    };

    // 操作成功( 默认 response 结果 )
    struct Success;
    using Success_s = std::shared_ptr<Success>;
    using Success_w = std::weak_ptr<Success>;

    // 出错( 通用 response 结果 )
    struct Error;
    using Error_s = std::shared_ptr<Error>;
    using Error_w = std::weak_ptr<Error>;

    // 操作成功( 默认 response 结果 )
    struct Success : xx::Object {

        typedef Success ThisType;
        typedef xx::Object BaseType;
	    Success() = default;
		Success(Success const&) = delete;
		Success& operator=(Success const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        void InitCascade() noexcept override;

        inline static std::shared_ptr<ThisType> defaultInstance;
    };
    // 出错( 通用 response 结果 )
    struct Error : xx::Object {
        int32_t id = 0;
        std::string_s txt;

        typedef Error ThisType;
        typedef xx::Object BaseType;
	    Error() = default;
		Error(Error const&) = delete;
		Error& operator=(Error const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        void InitCascade() noexcept override;

        inline static std::shared_ptr<ThisType> defaultInstance;
    };
}
namespace xx {
    template<> struct TypeId<PKG::Success> { static const uint16_t value = 3; };
    template<> struct TypeId<PKG::Error> { static const uint16_t value = 4; };
}
namespace PKG {
    inline uint16_t Success::GetTypeId() const noexcept {
        return 3;
    }
    inline void Success::ToBBuffer(xx::BBuffer& bb) const noexcept {
    }
    inline int Success::FromBBuffer(xx::BBuffer& bb) noexcept {
        return 0;
    }
    inline void Success::InitCascade() noexcept {
    }
    inline void Success::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"Success\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Success::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
    }
    inline uint16_t Error::GetTypeId() const noexcept {
        return 4;
    }
    inline void Error::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->id);
        bb.Write(this->txt);
    }
    inline int Error::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->id)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->txt)) return r;
        return 0;
    }
    inline void Error::InitCascade() noexcept {
    }
    inline void Error::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"Error\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Error::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"id\":", this->id);
        if (this->txt) xx::Append(s, ", \"txt\":\"", this->txt, "\"");
        else xx::Append(s, ", \"txt\":nil");
    }
}
namespace PKG {
	struct AllTypesRegister {
        AllTypesRegister() {
	        xx::BBuffer::Register<PKG::Success>(3);
	        xx::BBuffer::Register<PKG::Error>(4);
        }
	};
	inline AllTypesRegister AllTypesRegisterInstance;   // for auto register at program startup
}
