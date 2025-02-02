#pragma once

#include <cstdint>
#include <cassert>

#include "src/DebugToolDrivers/Protocols/RiscVDebug/DebugModule/DebugModule.hpp"

namespace DebugToolDrivers::Protocols::RiscVDebug::DebugModule::Registers
{
    struct ControlRegister
    {
        enum class HartSelectionMode: std::uint8_t
        {
            SINGLE = 0x00,
            MULTI = 0x01,
        };

        bool debugModuleActive = false;
        bool ndmReset = false;
        bool clearResetHaltRequest = false;
        bool setResetHaltRequest = false;
        bool clearKeepAlive = false;
        bool setKeepAlive = false;
        HartIndex selectedHartIndex = 0;
        HartSelectionMode hartSelectionMode = HartSelectionMode::SINGLE;
        bool acknowledgeUnavailableHarts = false;
        bool acknowledgeHaveReset = false;
        bool hartReset = false;
        bool resumeRequest = false;
        bool haltRequest = false;

        /**
         * These `fromValue()` functions cannot be constructors because we'd lose designated initialisation (as the
         * structs would no longer be aggregates - structs with user-defined constructors cannot be aggregates).
         *
         * @param value
         * @return
         */
        static constexpr auto fromValue(RegisterValue value) {
            return ControlRegister{
                .debugModuleActive = static_cast<bool>(value & 0x01),
                .ndmReset = static_cast<bool>(value & (0x01 << 1)),
                .clearResetHaltRequest = static_cast<bool>(value & (0x01 << 2)),
                .setResetHaltRequest = static_cast<bool>(value & (0x01 << 3)),
                .clearKeepAlive = static_cast<bool>(value & (0x01 << 4)),
                .setKeepAlive = static_cast<bool>(value & (0x01 << 5)),
                .selectedHartIndex = (((value >> 6) & 0x3FF) << 10) | ((value >> 16) & 0x3FF),
                .hartSelectionMode = static_cast<HartSelectionMode>(value & (0x01 << 26)),
                .acknowledgeUnavailableHarts = static_cast<bool>(value & (0x01 << 27)),
                .acknowledgeHaveReset = static_cast<bool>(value & (0x01 << 28)),
                .hartReset = static_cast<bool>(value & (0x01 << 29)),
                .resumeRequest = static_cast<bool>(value & (0x01 << 30)),
                .haltRequest = static_cast<bool>(value & static_cast<std::uint32_t>(0x01 << 31)),
            };
        }

        [[nodiscard]] constexpr RegisterValue value() const {
            assert(this->selectedHartIndex <= 0xFFFFF);

            return RegisterValue{0}
                | static_cast<RegisterValue>(this->debugModuleActive)
                | static_cast<RegisterValue>(this->ndmReset) << 1
                | static_cast<RegisterValue>(this->clearResetHaltRequest) << 2
                | static_cast<RegisterValue>(this->setResetHaltRequest) << 3
                | static_cast<RegisterValue>(this->clearKeepAlive) << 4
                | static_cast<RegisterValue>(this->setKeepAlive) << 5
                | static_cast<RegisterValue>((this->selectedHartIndex & 0xFFFFF) >> 10) << 6
                | static_cast<RegisterValue>(this->selectedHartIndex & 0x3FF) << 16
                | static_cast<RegisterValue>(this->hartSelectionMode) << 26
                | static_cast<RegisterValue>(this->acknowledgeUnavailableHarts) << 27
                | static_cast<RegisterValue>(this->acknowledgeHaveReset) << 28
                | static_cast<RegisterValue>(this->hartReset) << 29
                | static_cast<RegisterValue>(this->resumeRequest) << 30
                | static_cast<RegisterValue>(this->haltRequest) << 31
            ;
        }
    };
}
