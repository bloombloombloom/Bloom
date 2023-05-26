#pragma once

#include <cstdint>

#include "src/Targets/TargetMemory.hpp"

namespace Bloom::Targets::Microchip::Avr
{
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

    struct Fuse
    {
        FuseType type;
        std::uint8_t value;

        Fuse(FuseType type, std::uint8_t value)
            : type(type)
            , value(value)
        {}
    };

    struct FuseBitsDescriptor
    {
        TargetMemoryAddress byteAddress;

        /**
         * The type of the fuse byte in which the fuse bits resides.
         */
        FuseType fuseType;

        /**
         * Fuse bits mask
         */
        std::uint8_t bitMask;

        FuseBitsDescriptor(TargetMemoryAddress byteAddress, FuseType fuseType, std::uint8_t bitMask)
            : byteAddress(byteAddress)
            , fuseType(fuseType)
            , bitMask(bitMask)
        {}
    };
}
