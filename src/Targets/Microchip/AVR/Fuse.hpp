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

    struct FuseBitDescriptor
    {
        /**
         * The type of the fuse byte in which the fuse bit resides.
         */
        FuseType fuseType;

        /**
         * Fuse bit mask
         */
        std::uint8_t bitMask;

        FuseBitDescriptor(FuseType fuseType, std::uint8_t bitMask): fuseType(fuseType), bitMask(bitMask) {}
    };
}
