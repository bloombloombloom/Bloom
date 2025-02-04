#pragma once

#include "src/Targets/TargetBitFieldDescriptor.hpp"

namespace Targets::RiscV::Wch::Peripherals::Flash
{
    struct FlashStatusRegisterFields
    {
        const TargetBitFieldDescriptor& busy;
        const TargetBitFieldDescriptor& bootLock;
        const TargetBitFieldDescriptor& bootMode;
    };
}
