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

    struct FuseBitsDescriptor
    {
        /**
         * The type of the fuse byte in which the fuse bits resides.
         */
        FuseType fuseType;

        /**
         * Fuse bits mask
         */
        std::uint8_t bitMask;

        FuseBitsDescriptor(FuseType fuseType, std::uint8_t bitMask): fuseType(fuseType), bitMask(bitMask) {}
    };
}
