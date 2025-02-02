#pragma once

#include <cstdint>

#include "src/DebugToolDrivers/Protocols/RiscVDebug/Common.hpp"

namespace DebugToolDrivers::Protocols::RiscVDebug::Registers
{
    struct DebugControlStatusRegister
    {
        enum class DebugModeCause: std::uint8_t
        {
            BREAK = 0x01,
            TRIGGER = 0x02,
            HALT_REQUEST = 0x03,
            STEP = 0x04,
            RESET_HALT_REQUEST = 0x05,
            GROUP = 0x06,
        };

        PrivilegeMode privilegeMode = PrivilegeMode::USER;
        bool step = false;
        bool nmiPending = false;
        bool mprvEnabled = false;
        DebugModeCause debugModeCause = DebugModeCause::BREAK;
        bool stopTime = false;
        bool stopCount = false;
        bool stepInterruptsEnabled = false;
        bool breakUMode = false;
        bool breakSMode = false;
        bool breakMMode = false;
        bool breakVUMode = false;
        bool breakVSMode = false;
        std::uint8_t debugVersion = 0;

        static constexpr auto fromValue(RegisterValue value) {
            return DebugControlStatusRegister{
                .privilegeMode = static_cast<PrivilegeMode>(value & 0x03),
                .step = static_cast<bool>(value & (0x01 << 2)),
                .nmiPending = static_cast<bool>(value & (0x01 << 3)),
                .mprvEnabled = static_cast<bool>(value & (0x01 << 4)),
                .debugModeCause = static_cast<DebugModeCause>((value >> 6) & 0x07),
                .stopTime = static_cast<bool>(value & (0x01 << 9)),
                .stopCount = static_cast<bool>(value & (0x01 << 10)),
                .stepInterruptsEnabled = static_cast<bool>(value & (0x01 << 11)),
                .breakUMode = static_cast<bool>(value & (0x01 << 12)),
                .breakSMode = static_cast<bool>(value & (0x01 << 13)),
                .breakMMode = static_cast<bool>(value & (0x01 << 15)),
                .breakVUMode = static_cast<bool>(value & (0x01 << 16)),
                .breakVSMode = static_cast<bool>(value & (0x01 << 17)),
                .debugVersion = static_cast<std::uint8_t>((value >> 28) & 0x0F)
            };
        }

        [[nodiscard]] constexpr RegisterValue value() const {
            return RegisterValue{0}
                | static_cast<RegisterValue>(this->privilegeMode)
                | static_cast<RegisterValue>(this->step) << 2
                | static_cast<RegisterValue>(this->nmiPending) << 3
                | static_cast<RegisterValue>(this->mprvEnabled) << 4
                | static_cast<RegisterValue>(this->debugModeCause) << 6
                | static_cast<RegisterValue>(this->stopTime) << 9
                | static_cast<RegisterValue>(this->stopCount) << 10
                | static_cast<RegisterValue>(this->stepInterruptsEnabled) << 11
                | static_cast<RegisterValue>(this->breakUMode) << 12
                | static_cast<RegisterValue>(this->breakSMode) << 13
                | static_cast<RegisterValue>(this->breakMMode) << 15
                | static_cast<RegisterValue>(this->breakVUMode) << 16
                | static_cast<RegisterValue>(this->breakVSMode) << 17
                | static_cast<RegisterValue>(this->debugVersion & 0x0F) << 28
            ;
        }
    };
}
