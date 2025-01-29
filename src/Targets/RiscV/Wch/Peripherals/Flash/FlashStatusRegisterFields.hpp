#pragma once

#include "src/Targets/TargetBitFieldDescriptor.hpp"
#include "src/Targets/RiscV/Wch/TargetDescriptionFile.hpp"

namespace Targets::RiscV::Wch::Peripherals::Flash
{
    struct FlashStatusRegisterFields
    {
        const TargetBitFieldDescriptor& busy;
        const TargetBitFieldDescriptor& bootLock;
        const TargetBitFieldDescriptor& bootMode;
    };
}
