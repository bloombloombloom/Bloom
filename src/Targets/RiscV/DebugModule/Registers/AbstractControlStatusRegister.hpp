#pragma once

#include <cstdint>

#include "src/Targets/RiscV/DebugModule/DebugModule.hpp"

namespace Targets::RiscV::DebugModule::Registers
{
    struct AbstractControlStatusRegister
    {
        enum CommandError: std::uint8_t
        {
            NONE = 0x00,
            BUSY = 0x01,
            NOT_SUPPORTED = 0x02,
            EXCEPTION = 0x03,
            HALT_RESUME = 0x04,
            BUS = 0x05,
            OTHER = 0x07,
        };

        std::uint8_t dataCount:4 = 0;
        CommandError commandError:3 = CommandError::NONE;
        bool relaxedPrivilege:1 = false;
        bool busy:1 = false;
        std::uint8_t programBufferSize:5 = 0;

        AbstractControlStatusRegister() = default;

        constexpr explicit AbstractControlStatusRegister(RegisterValue registerValue)
            : dataCount(static_cast<std::uint8_t>(registerValue & 0x0F))
            , commandError(static_cast<CommandError>((registerValue >> 8) & 0x07))
            , relaxedPrivilege(static_cast<bool>(registerValue & (0x01 << 11)))
            , busy(static_cast<bool>(registerValue & (0x01 << 12)))
            , programBufferSize(static_cast<std::uint8_t>((registerValue >> 24) & 0x1F))
        {}

        constexpr RegisterValue value() const {
            return RegisterValue{0}
                | static_cast<RegisterValue>(this->dataCount)
                | static_cast<RegisterValue>(this->commandError) << 8
                | static_cast<RegisterValue>(this->relaxedPrivilege) << 11
                | static_cast<RegisterValue>(this->busy) << 12
                | static_cast<RegisterValue>(this->programBufferSize) << 24
            ;
        }

        constexpr void clearCommandError() {
            // Setting all of the bits will clear the field
            this->commandError = CommandError::OTHER;
        }
    };
}
