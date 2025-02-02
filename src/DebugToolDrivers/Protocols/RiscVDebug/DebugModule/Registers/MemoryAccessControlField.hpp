#pragma once

#include <cstdint>

#include "src/DebugToolDrivers/Protocols/RiscVDebug/Common.hpp"

namespace DebugToolDrivers::Protocols::RiscVDebug::DebugModule::Registers
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

        bool write = false;
        bool postIncrement = false;
        MemorySize size = MemorySize::SIZE_8;
        bool virtualAddress = false;

        static constexpr auto fromValue(std::uint32_t value) {
            return MemoryAccessControlField{
                .write = static_cast<bool>(value & (0x01 << 16)),
                .postIncrement = static_cast<bool>(value & (0x01 << 19)),
                .size = static_cast<MemorySize>((value >> 20) & 0x07),
                .virtualAddress = static_cast<bool>(value & (0x01 << 23)),
            };
        }

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
