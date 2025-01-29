#pragma once

#include "src/Targets/TargetBitFieldDescriptor.hpp"
#include "src/Targets/RiscV/Wch/TargetDescriptionFile.hpp"

namespace Targets::RiscV::Wch::Peripherals::Flash
{
    struct FlashControlRegisterFields
    {
        const TargetBitFieldDescriptor& locked;
        const TargetBitFieldDescriptor& startErase;
        const TargetBitFieldDescriptor& mainSegmentErase;
    };
}
