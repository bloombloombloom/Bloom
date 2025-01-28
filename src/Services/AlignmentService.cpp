#include "AlignmentService.hpp"

#include <cmath>

namespace Services
{
    Targets::TargetMemoryAddress AlignmentService::alignMemoryAddress(
        Targets::TargetMemoryAddress address,
        Targets::TargetMemorySize alignTo
    ) {
        return (address / alignTo) * alignTo;
    }

    Targets::TargetMemorySize AlignmentService::alignMemorySize(
        Targets::TargetMemorySize size,
        Targets::TargetMemorySize alignTo
    ) {
        return (size % alignTo) != 0
            ? static_cast<Targets::TargetMemorySize>(
                std::ceil(static_cast<double>(size) / static_cast<double>(alignTo)) * alignTo
            )
            : size;
    }

    Targets::TargetMemoryAddressRange AlignmentService::alignAddressRange(
        const Targets::TargetMemoryAddressRange& from,
        Targets::TargetMemorySize alignTo
    ) {
        const auto alignedStartAddress = AlignmentService::alignMemoryAddress(from.startAddress, alignTo);
        return Targets::TargetMemoryAddressRange{
            alignedStartAddress,
            alignedStartAddress + AlignmentService::alignMemorySize(from.size()
                + (from.startAddress - alignedStartAddress), alignTo) - 1
        };
    }
}
