#pragma once

#include <cstdint>
#include <vector>

namespace Bloom::Targets
{
    enum class TargetMemoryType: std::uint8_t
    {
        FLASH,
        RAM,
        EEPROM,
        OTHER,
    };

    struct TargetMemoryAddressRange
    {
        std::uint32_t startAddress = 0;
        std::uint32_t endAddress = 0;

        TargetMemoryAddressRange() = default;
        TargetMemoryAddressRange(std::uint32_t startAddress, std::uint32_t endAddress)
        : startAddress(startAddress), endAddress(endAddress) {};

        [[nodiscard]] bool intersectsWith(const TargetMemoryAddressRange& other) const {
            return
                (other.startAddress <= this->startAddress && other.endAddress >= this->startAddress)
                || (other.endAddress >= this->endAddress && other.startAddress <= this->endAddress)
            ;
        }
    };

    struct TargetMemoryDescriptor
    {
        TargetMemoryType type;
        TargetMemoryAddressRange addressRange;

        TargetMemoryDescriptor(TargetMemoryType type, TargetMemoryAddressRange addressRange)
        : type(type), addressRange(addressRange) {};

        [[nodiscard]] std::uint32_t size() const {
            return (this->addressRange.endAddress - this->addressRange.startAddress) + 1;
        }
    };

    using TargetMemoryBuffer = std::vector<unsigned char>;
}
