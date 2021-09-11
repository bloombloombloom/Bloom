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
    };

    using TargetMemoryBuffer = std::vector<unsigned char>;
}
