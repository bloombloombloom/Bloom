#pragma once

#include "src/Targets/TargetMemory.hpp"
#include "src/Targets/TargetMemoryAddressRange.hpp"

namespace Services
{
    class AlignmentService
    {
    public:
        static Targets::TargetMemoryAddress alignMemoryAddress(
            Targets::TargetMemoryAddress address,
            Targets::TargetMemorySize alignTo
        );

        static Targets::TargetMemorySize alignMemorySize(
            Targets::TargetMemorySize size,
            Targets::TargetMemorySize alignTo
        );

        static Targets::TargetMemoryAddressRange alignAddressRange(
            const Targets::TargetMemoryAddressRange& from,
            Targets::TargetMemorySize alignTo
        );
    };
}
