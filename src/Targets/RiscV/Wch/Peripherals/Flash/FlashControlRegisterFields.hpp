#pragma once

#include "src/Targets/TargetBitFieldDescriptor.hpp"

namespace Targets::RiscV::Wch::Peripherals::Flash
{
    struct FlashControlRegisterFields
    {
        const TargetBitFieldDescriptor& locked;
        const TargetBitFieldDescriptor& startErase;
        const TargetBitFieldDescriptor& mainSegmentErase;
    };
}
