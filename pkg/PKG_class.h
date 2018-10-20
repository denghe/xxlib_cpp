#pragma once
#include "xx.h"

namespace PKG
{
	struct PkgGenMd5
	{
		static constexpr char const* value = "ce880f00e683dc3af6007408bf5d85f1";
    };

    // 操作成功( 默认 response 结果 )
    class Success;
    using Success_p = xx::Ptr<Success>;
    using Success_r = xx::Ref<Success>;

    // 出错( 通用 response 结果 )
    class Error;
    using Error_p = xx::Ptr<Error>;
    using Error_r = xx::Ref<Error>;

namespace Server
{
    // 服务间互表身份的首包
    class Info;
    using Info_p = xx::Ptr<Info>;
    using Info_r = xx::Ref<Info>;

}
namespace Server
{
    enum class Types : int32_t
    {
        Unknown = 0,
        Login = 1,
        Lobby = 2,
        DB = 3,
        MAX = 4,
    };
}
    // 操作成功( 默认 response 结果 )
    class Success : public xx::Object
    {
    public:

        typedef Success ThisType;
        typedef xx::Object BaseType;
	    Success(xx::MemPool* const& mempool) noexcept;
	    Success(xx::BBuffer* const& bb);
		Success(Success const&) = delete;
		Success& operator=(Success const&) = delete;
        void ToString(xx::String& s) const noexcept override;
        void ToStringCore(xx::String& s) const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;
        void CopyTo(Success* const& o) const noexcept;
        Success* MakeCopy() const noexcept;
        Success_p MakePtrCopy() const noexcept;
        inline static xx::Ptr<ThisType> defaultInstance;
    };
    // 出错( 通用 response 结果 )
    class Error : public xx::Object
    {
    public:
        int32_t id = 0;
        xx::String_p txt;

        typedef Error ThisType;
        typedef xx::Object BaseType;
	    Error(xx::MemPool* const& mempool) noexcept;
	    Error(xx::BBuffer* const& bb);
		Error(Error const&) = delete;
		Error& operator=(Error const&) = delete;
        void ToString(xx::String& s) const noexcept override;
        void ToStringCore(xx::String& s) const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;
        void CopyTo(Error* const& o) const noexcept;
        Error* MakeCopy() const noexcept;
        Error_p MakePtrCopy() const noexcept;
        inline static xx::Ptr<ThisType> defaultInstance;
    };
namespace Server
{
    // 服务间互表身份的首包
    class Info : public xx::Object
    {
    public:
        PKG::Server::Types type = (PKG::Server::Types)0;

        typedef Info ThisType;
        typedef xx::Object BaseType;
	    Info(xx::MemPool* const& mempool) noexcept;
	    Info(xx::BBuffer* const& bb);
		Info(Info const&) = delete;
		Info& operator=(Info const&) = delete;
        void ToString(xx::String& s) const noexcept override;
        void ToStringCore(xx::String& s) const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;
        void CopyTo(Info* const& o) const noexcept;
        Info* MakeCopy() const noexcept;
        Info_p MakePtrCopy() const noexcept;
        inline static xx::Ptr<ThisType> defaultInstance;
    };
}
}
namespace xx
{
	template<> struct TypeId<PKG::Success> { static const uint16_t value = 3; };
	template<> struct TypeId<PKG::Error> { static const uint16_t value = 4; };
	template<> struct TypeId<PKG::Server::Info> { static const uint16_t value = 5; };
}
namespace PKG
{
	inline Success::Success(xx::MemPool* const& mempool) noexcept
        : xx::Object(mempool)
	{
	}
	inline Success::Success(xx::BBuffer* const& bb)
        : xx::Object(bb)
	{
        if (int r = FromBBufferCore(*bb)) throw r;
	}
    inline void Success::ToBBuffer(xx::BBuffer& bb) const noexcept
    {
    }
    inline int Success::FromBBuffer(xx::BBuffer& bb) noexcept
    {
        return this->FromBBufferCore(bb);
    }
    inline int Success::FromBBufferCore(xx::BBuffer& bb) noexcept
    {
        return 0;
    }

    inline void Success::ToString(xx::String& s) const noexcept
    {
        if (this->memHeader().flags)
        {
        	s.Append("[ \"***** recursived *****\" ]");
        	return;
        }
        else this->memHeader().flags = 1;

        s.Append("{ \"pkgTypeName\":\"Success\", \"pkgTypeId\":", xx::TypeId_v<ThisType>);
        ToStringCore(s);
        s.Append(" }");
        
        this->memHeader().flags = 0;
    }
    inline void Success::ToStringCore(xx::String& s) const noexcept
    {
        this->BaseType::ToStringCore(s);
    }
    inline void Success::CopyTo(Success* const& o) const noexcept
    {
    }
    inline Success* Success::MakeCopy() const noexcept
    {
        auto o = mempool->MPCreate<Success>();
        this->CopyTo(o);
        return o;
    }
    inline Success_p Success::MakePtrCopy() const noexcept
    {
        return Success_p(this->MakeCopy());
    }

	inline Error::Error(xx::MemPool* const& mempool) noexcept
        : xx::Object(mempool)
	{
	}
	inline Error::Error(xx::BBuffer* const& bb)
        : xx::Object(bb)
	{
        if (int r = FromBBufferCore(*bb)) throw r;
	}
    inline void Error::ToBBuffer(xx::BBuffer& bb) const noexcept
    {
        bb.Write(this->id);
        bb.Write(this->txt);
    }
    inline int Error::FromBBuffer(xx::BBuffer& bb) noexcept
    {
        return this->FromBBufferCore(bb);
    }
    inline int Error::FromBBufferCore(xx::BBuffer& bb) noexcept
    {
        if (int r = bb.Read(this->id)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->txt)) return r;
        return 0;
    }

    inline void Error::ToString(xx::String& s) const noexcept
    {
        if (this->memHeader().flags)
        {
        	s.Append("[ \"***** recursived *****\" ]");
        	return;
        }
        else this->memHeader().flags = 1;

        s.Append("{ \"pkgTypeName\":\"Error\", \"pkgTypeId\":", xx::TypeId_v<ThisType>);
        ToStringCore(s);
        s.Append(" }");
        
        this->memHeader().flags = 0;
    }
    inline void Error::ToStringCore(xx::String& s) const noexcept
    {
        this->BaseType::ToStringCore(s);
        s.Append(", \"id\":", this->id);
        if (this->txt) s.Append(", \"txt\":\"", this->txt, "\"");
        else s.Append(", \"txt\":nil");
    }
    inline void Error::CopyTo(Error* const& o) const noexcept
    {
        o->id = this->id;
        o->txt = this->txt;
    }
    inline Error* Error::MakeCopy() const noexcept
    {
        auto o = mempool->MPCreate<Error>();
        this->CopyTo(o);
        return o;
    }
    inline Error_p Error::MakePtrCopy() const noexcept
    {
        return Error_p(this->MakeCopy());
    }

namespace Server
{
	inline Info::Info(xx::MemPool* const& mempool) noexcept
        : xx::Object(mempool)
	{
	}
	inline Info::Info(xx::BBuffer* const& bb)
        : xx::Object(bb)
	{
        if (int r = FromBBufferCore(*bb)) throw r;
	}
    inline void Info::ToBBuffer(xx::BBuffer& bb) const noexcept
    {
        bb.Write(this->type);
    }
    inline int Info::FromBBuffer(xx::BBuffer& bb) noexcept
    {
        return this->FromBBufferCore(bb);
    }
    inline int Info::FromBBufferCore(xx::BBuffer& bb) noexcept
    {
        if (int r = bb.Read(this->type)) return r;
        return 0;
    }

    inline void Info::ToString(xx::String& s) const noexcept
    {
        if (this->memHeader().flags)
        {
        	s.Append("[ \"***** recursived *****\" ]");
        	return;
        }
        else this->memHeader().flags = 1;

        s.Append("{ \"pkgTypeName\":\"Server.Info\", \"pkgTypeId\":", xx::TypeId_v<ThisType>);
        ToStringCore(s);
        s.Append(" }");
        
        this->memHeader().flags = 0;
    }
    inline void Info::ToStringCore(xx::String& s) const noexcept
    {
        this->BaseType::ToStringCore(s);
        s.Append(", \"type\":", this->type);
    }
    inline void Info::CopyTo(Info* const& o) const noexcept
    {
        o->type = this->type;
    }
    inline Info* Info::MakeCopy() const noexcept
    {
        auto o = mempool->MPCreate<Info>();
        this->CopyTo(o);
        return o;
    }
    inline Info_p Info::MakePtrCopy() const noexcept
    {
        return Info_p(this->MakeCopy());
    }

}
}
namespace PKG
{
	inline void AllTypesRegister() noexcept
	{
        xx::MemPool::RegisterInternals();
	    xx::MemPool::Register<PKG::Success, xx::Object>();
	    xx::MemPool::Register<PKG::Error, xx::Object>();
	    xx::MemPool::Register<PKG::Server::Info, xx::Object>();
	}
}
