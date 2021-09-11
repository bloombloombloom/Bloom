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

    using TargetMemoryBuffer = std::vector<unsigned char>;
}
