#pragma once
#include "xx_bbuffer.h"
namespace PKG {
	struct PkgGenMd5 {
		inline static const std::string value = "616f8410ebbcc955a8038c36ddfb65f2";
    };
	struct AllTypesRegister {
        AllTypesRegister();
    };
    inline AllTypesRegister allTypesRegisterInstance;   // for auto register at program startup

namespace Generic {
    struct Xxx;
    using Xxx_s = std::shared_ptr<Xxx>;
    using Xxx_w = std::weak_ptr<Xxx>;

}
namespace Generic {
    struct Xxx : xx::Object {
        std::optional<int32_t> ticks;

        XX_CODEGEN_CLASS_HEADER_CASCADE(Xxx, xx::Object)
    };
}
}
namespace xx {
    template<> struct TypeId<PKG::Generic::Xxx> { static const uint16_t value = 7; };
}
