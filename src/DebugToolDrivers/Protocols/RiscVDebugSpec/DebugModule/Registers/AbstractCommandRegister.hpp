#pragma once

#include <cstdint>
#include <cassert>

#include "src/DebugToolDrivers/Protocols/RiscVDebugSpec/DebugModule/DebugModule.hpp"

namespace DebugToolDrivers::Protocols::RiscVDebugSpec::DebugModule::Registers
{
    struct AbstractCommandRegister
    {
        enum class CommandType: std::uint8_t
        {
            REGISTER_ACCESS = 0x00,
            QUICK_ACCESS = 0x01,
            MEMORY_ACCESS = 0x02,
        };

        std::uint32_t control;
        CommandType commandType;

        static constexpr AbstractCommandRegister fromValue(RegisterValue value) {
            return {
                .control = static_cast<std::uint32_t>(value & 0x00FFFFFF),
                .commandType = static_cast<CommandType>((value >> 24) & 0xFF),
            };
        }

        [[nodiscard]] constexpr RegisterValue value() const {
            assert(this->control <= 0x00FFFFFF);

            return RegisterValue{0}
                | static_cast<RegisterValue>(this->control & 0x00FFFFFF)
                | static_cast<RegisterValue>(this->commandType) << 24
            ;
        }
    };
}
