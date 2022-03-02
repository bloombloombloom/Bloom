#pragma once

#include <cstdint>

namespace Bloom::Targets::Microchip::Avr
{
    enum class FuseType: std::uint8_t
    {
        LOW,
        HIGH,
        EXTENDED,
    };

    struct Fuse
    {
        FuseType type;
        std::uint8_t value;

        Fuse(FuseType type, std::uint8_t value): type(type), value(value) {}
    };
}
