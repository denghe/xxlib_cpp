#include "PKG_class.h"
namespace PKG {
namespace Generic {
    uint16_t Xxx::GetTypeId() const noexcept {
        return 7;
    }
    void Xxx::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->ticks);
    }
    int Xxx::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->ticks)) return r;
        return 0;
    }
    int Xxx::InitCascade(void* const& o) noexcept {
        return 0;
    }
    void Xxx::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"Generic.Xxx\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    void Xxx::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"ticks\":", this->ticks);
    }
}
}
namespace PKG {
	AllTypesRegister::AllTypesRegister() {
	    xx::BBuffer::Register<PKG::Generic::Xxx>(7);
	}
}
