#pragma once

#include <cstdint>

#include "src/DebugToolDrivers/Protocols/RiscVDebugSpec/DebugModule/DebugModule.hpp"

namespace DebugToolDrivers::Protocols::RiscVDebugSpec::DebugModule::Registers
{
    struct AbstractControlStatusRegister
    {
        std::uint8_t dataCount:4;
        AbstractCommandError commandError:3;
        bool relaxedPrivilege:1;
        bool busy:1;
        std::uint8_t programBufferSize:5;

        constexpr explicit AbstractControlStatusRegister(RegisterValue registerValue)
            : dataCount(static_cast<std::uint8_t>(registerValue & 0x0F))
            , commandError(static_cast<AbstractCommandError>((registerValue >> 8) & 0x07))
            , relaxedPrivilege(static_cast<bool>(registerValue & (0x01 << 11)))
            , busy(static_cast<bool>(registerValue & (0x01 << 12)))
            , programBufferSize(static_cast<std::uint8_t>((registerValue >> 24) & 0x1F))
        {}

        [[nodiscard]] constexpr RegisterValue value() const {
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
            this->commandError = AbstractCommandError::OTHER;
        }
    };
}
