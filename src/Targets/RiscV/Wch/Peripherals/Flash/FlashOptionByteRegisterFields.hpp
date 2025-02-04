#pragma once

#include "src/Targets/TargetBitFieldDescriptor.hpp"

namespace Targets::RiscV::Wch::Peripherals::Flash
{
    struct FlashOptionByteRegisterFields
    {
        const TargetBitFieldDescriptor& readProtected;
    };
}
