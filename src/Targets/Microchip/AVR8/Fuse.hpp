#pragma once

#include <cstdint>

#include "src/Targets/TargetMemory.hpp"

namespace Targets::Microchip::Avr8
{
    using FuseValue = std::uint8_t;

    enum class FuseType: std::uint8_t
    {
        LOW,
        HIGH,
        EXTENDED,
        OTHER,
    };

    enum class FuseEnableStrategy: std::uint8_t
    {
        CLEAR,
        SET,
    };
}
