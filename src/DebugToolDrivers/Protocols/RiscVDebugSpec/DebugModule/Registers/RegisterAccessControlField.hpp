#pragma once

#include <cstdint>

#include "src/DebugToolDrivers/Protocols/RiscVDebugSpec/Common.hpp"

namespace DebugToolDrivers::Protocols::RiscVDebugSpec::DebugModule::Registers
{
    struct RegisterAccessControlField
    {
        struct Flags
        {
            bool postExecute = false;
            bool postIncrement = false;
        };

        enum class RegisterSize: std::uint8_t
        {
            SIZE_32 = 0x02,
            SIZE_64 = 0x03,
            SIZE_128 = 0x04,
        };

        RegisterNumber registerNumber = 0;
        bool write = false;
        bool transfer = false;
        Flags flags = {};
        RegisterSize size = RegisterSize::SIZE_32;

        static constexpr auto fromValue(std::uint32_t value) {
            return RegisterAccessControlField{
                .registerNumber = static_cast<RegisterNumber>(value & 0xFFFF),
                .write = static_cast<bool>(value & (0x01 << 16)),
                .transfer = static_cast<bool>(value & (0x01 << 17)),
                .flags = {
                    .postExecute = static_cast<bool>(value & (0x01 << 18)),
                    .postIncrement = static_cast<bool>(value & (0x01 << 19)),
                },
                .size = static_cast<RegisterSize>((value >> 20) & 0x07),
            };
        }

        [[nodiscard]] constexpr std::uint32_t value() const {
            return std::uint32_t{0}
                | static_cast<std::uint32_t>(this->registerNumber)
                | static_cast<std::uint32_t>(this->write) << 16
                | static_cast<std::uint32_t>(this->transfer) << 17
                | static_cast<std::uint32_t>(this->flags.postExecute) << 18
                | static_cast<std::uint32_t>(this->flags.postIncrement) << 19
                | static_cast<std::uint32_t>(this->size) << 20
            ;
        }
    };
}
