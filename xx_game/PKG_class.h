#pragma once
#include "xx_bbuffer.h"
namespace PKG {
	struct PkgGenMd5 {
		inline static const std::string value = "5a979261b82493626e7adfc858139d8e";
    };
	struct AllTypesRegister {
        AllTypesRegister();
    };
    inline AllTypesRegister allTypesRegisterInstance;   // for auto register at program startup

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
