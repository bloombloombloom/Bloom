#pragma once

#include <cstdint>

#include "src/DebugToolDrivers/Protocols/RiscVDebugSpec/DebugModule/DebugModule.hpp"

namespace DebugToolDrivers::Protocols::RiscVDebugSpec::DebugModule::Registers
{
    struct StatusRegister
    {
        std::uint8_t version = 0;
        bool validConfigStructurePointer = false;
        bool supportsResetHalt = false;
        bool authBusy = false;
        bool authenticated = false;
        bool anyHalted = false;
        bool allHalted = false;
        bool anyRunning = false;
        bool allRunning = false;
        bool anyUnavailable = false;
        bool allUnavailable = false;
        bool anyNonExistent = false;
        bool allNonExistent = false;
        bool anyResumeAcknowledge = false;
        bool allResumeAcknowledge = false;
        bool anyHaveReset = false;
        bool allHaveReset = false;
        bool implicitBreak = false;
        bool stickyUnavailableBits = false;
        bool ndmResetPending = false;

        static constexpr StatusRegister fromValue(RegisterValue value) {
            return {
                .version = static_cast<std::uint8_t>(value & 0x0F),
                .validConfigStructurePointer = static_cast<bool>(value & (0x01 << 4)),
                .supportsResetHalt = static_cast<bool>(value & (0x01 << 5)),
                .authBusy = static_cast<bool>(value & (0x01 << 6)),
                .authenticated = static_cast<bool>(value & (0x01 << 7)),
                .anyHalted = static_cast<bool>(value & (0x01 << 8)),
                .allHalted = static_cast<bool>(value & (0x01 << 9)),
                .anyRunning = static_cast<bool>(value & (0x01 << 10)),
                .allRunning = static_cast<bool>(value & (0x01 << 11)),
                .anyUnavailable = static_cast<bool>(value & (0x01 << 12)),
                .allUnavailable = static_cast<bool>(value & (0x01 << 13)),
                .anyNonExistent = static_cast<bool>(value & (0x01 << 14)),
                .allNonExistent = static_cast<bool>(value & (0x01 << 15)),
                .anyResumeAcknowledge = static_cast<bool>(value & (0x01 << 16)),
                .allResumeAcknowledge = static_cast<bool>(value & (0x01 << 17)),
                .anyHaveReset = static_cast<bool>(value & (0x01 << 18)),
                .allHaveReset = static_cast<bool>(value & (0x01 << 19)),
                .implicitBreak = static_cast<bool>(value & (0x01 << 22)),
                .stickyUnavailableBits = static_cast<bool>(value & (0x01 << 23)),
                .ndmResetPending = static_cast<bool>(value & (0x01 << 24)),
            };
        }

        [[nodiscard]] constexpr RegisterValue value() const {
            return RegisterValue{0}
                | static_cast<RegisterValue>(this->version & 0x0F)
                | static_cast<RegisterValue>(this->validConfigStructurePointer) << 4
                | static_cast<RegisterValue>(this->supportsResetHalt) << 5
                | static_cast<RegisterValue>(this->authBusy) << 6
                | static_cast<RegisterValue>(this->authenticated) << 7
                | static_cast<RegisterValue>(this->anyHalted) << 8
                | static_cast<RegisterValue>(this->allHalted) << 9
                | static_cast<RegisterValue>(this->anyRunning) << 10
                | static_cast<RegisterValue>(this->allRunning) << 11
                | static_cast<RegisterValue>(this->anyUnavailable) << 12
                | static_cast<RegisterValue>(this->allUnavailable) << 13
                | static_cast<RegisterValue>(this->anyNonExistent) << 14
                | static_cast<RegisterValue>(this->allNonExistent) << 15
                | static_cast<RegisterValue>(this->anyResumeAcknowledge) << 16
                | static_cast<RegisterValue>(this->allResumeAcknowledge) << 17
                | static_cast<RegisterValue>(this->anyHaveReset) << 18
                | static_cast<RegisterValue>(this->allHaveReset) << 19
                | static_cast<RegisterValue>(this->implicitBreak) << 22
                | static_cast<RegisterValue>(this->stickyUnavailableBits) << 23
                | static_cast<RegisterValue>(this->ndmResetPending) << 24
            ;
        }
    };
}
