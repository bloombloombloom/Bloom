#include "MemoryRegion.hpp"

using namespace Bloom;

using Bloom::Targets::TargetMemoryAddressRange;

TargetMemoryAddressRange MemoryRegion::getAbsoluteAddressRange() const {
    if (this->addressRangeType == MemoryRegionAddressType::ABSOLUTE) {
        return this->addressRange;
    }

    return TargetMemoryAddressRange(
        this->addressRange.startAddress + this->memoryDescriptor.addressRange.startAddress,
        this->addressRange.endAddress + this->memoryDescriptor.addressRange.startAddress
    );
}

TargetMemoryAddressRange MemoryRegion::getRelativeAddressRange() const {
    if (this->addressRangeType == MemoryRegionAddressType::RELATIVE) {
        return this->addressRange;
    }

    return TargetMemoryAddressRange(
        this->addressRange.startAddress - this->memoryDescriptor.addressRange.startAddress,
        this->addressRange.endAddress - this->memoryDescriptor.addressRange.startAddress
    );
}
