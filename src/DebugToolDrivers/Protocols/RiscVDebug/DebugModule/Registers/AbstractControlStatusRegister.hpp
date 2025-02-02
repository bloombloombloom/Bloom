#pragma once

#include <cstdint>

#include "src/DebugToolDrivers/Protocols/RiscVDebug/DebugModule/DebugModule.hpp"

namespace DebugToolDrivers::Protocols::RiscVDebug::DebugModule::Registers
{
    struct AbstractControlStatusRegister
    {
        std::uint8_t dataRegisterCount = 0;
        AbstractCommandError commandError = AbstractCommandError::NONE;
        bool relaxedPrivilege = false;
        bool busy = false;
        std::uint8_t programBufferSize = 0;

        static constexpr AbstractControlStatusRegister fromValue(RegisterValue value) {
            return {
                .dataRegisterCount = static_cast<std::uint8_t>(value & 0x0F),
                .commandError = static_cast<AbstractCommandError>((value >> 8) & 0x07),
                .relaxedPrivilege = static_cast<bool>(value & (0x01 << 11)),
                .busy = static_cast<bool>(value & (0x01 << 12)),
                .programBufferSize = static_cast<std::uint8_t>((value >> 24) & 0x1F),
            };
        }

        [[nodiscard]] constexpr RegisterValue value() const {
            return RegisterValue{0}
                | static_cast<RegisterValue>(this->dataRegisterCount & 0x0F)
                | static_cast<RegisterValue>(this->commandError) << 8
                | static_cast<RegisterValue>(this->relaxedPrivilege) << 11
                | static_cast<RegisterValue>(this->busy) << 12
                | static_cast<RegisterValue>(this->programBufferSize & 0x1F) << 24
            ;
        }
    };
}
