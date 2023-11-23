#pragma once

#include <cstdint>
#include <cassert>

#include "src/Targets/RiscV/DebugModule/DebugModule.hpp"

namespace Targets::RiscV::DebugModule::Registers
{
    struct AbstractCommandRegister
    {
        enum CommandType: std::uint8_t
        {
            REGISTER_ACCESS = 0x00,
            QUICK_ACCESS = 0x01,
            MEMORY_ACCESS = 0x02,
        };

        std::uint32_t control = 0;
        CommandType commandType = CommandType::REGISTER_ACCESS;

        AbstractCommandRegister() = default;

        constexpr explicit AbstractCommandRegister(RegisterValue registerValue)
            : control(static_cast<std::uint32_t>(registerValue & 0x00FFFFFF))
            , commandType(static_cast<CommandType>((registerValue >> 24) & 0xFF))
        {}

        constexpr RegisterValue value() const {
            assert(this->control <= 0x00FFFFFF);

            return RegisterValue{0}
                | static_cast<RegisterValue>(this->control & 0x00FFFFFF)
                | static_cast<RegisterValue>(this->commandType) << 24
            ;
        }
    };
}
