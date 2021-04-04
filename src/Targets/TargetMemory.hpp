#pragma once

#include <vector>

namespace Bloom::Targets
{
    using TargetMemoryBuffer = std::vector<unsigned char>;

    enum class TargetMemoryType: unsigned int
    {
        FLASH,
        RAM,
        EEPROM,
    };
}
