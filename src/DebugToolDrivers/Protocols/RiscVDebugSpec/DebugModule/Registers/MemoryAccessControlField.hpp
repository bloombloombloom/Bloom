#pragma once

#include <cstdint>

#include "src/DebugToolDrivers/Protocols/RiscVDebugSpec/RiscVGeneric.hpp"

namespace DebugToolDrivers::Protocols::RiscVDebugSpec::DebugModule::Registers
{
    struct MemoryAccessControlField
    {
        enum class MemorySize: std::uint8_t
        {
            SIZE_8 = 0x00,
            SIZE_16 = 0x01,
            SIZE_32 = 0x02,
            SIZE_64 = 0x03,
            SIZE_128 = 0x04,
        };

        bool write:1 = false;
        bool postIncrement:1 = false;
        MemorySize size:3 = MemorySize::SIZE_32;
        bool virtualAddress:1 = false;

        constexpr MemoryAccessControlField() = default;

        constexpr MemoryAccessControlField(
            bool write,
            bool postIncrement,
            MemorySize size,
            bool virtualAddress
        )
            : write(write)
            , postIncrement(postIncrement)
            , size(size)
            , virtualAddress(virtualAddress)
        {}

        constexpr explicit MemoryAccessControlField(std::uint32_t controlValue)
            : write(static_cast<bool>(controlValue & (0x01 << 16)))
            , postIncrement(static_cast<bool>(controlValue & (0x01 << 19)))
            , size(static_cast<MemorySize>((controlValue >> 20) & 0x07))
            , virtualAddress(static_cast<bool>(controlValue & (0x01 << 23)))
        {}

        [[nodiscard]] constexpr std::uint32_t value() const {
            return std::uint32_t{0}
                | static_cast<std::uint32_t>(this->write) << 16
                | static_cast<std::uint32_t>(this->postIncrement) << 19
                | static_cast<std::uint32_t>(this->size) << 20
                | static_cast<std::uint32_t>(this->virtualAddress) << 23
            ;
        }
    };
}
