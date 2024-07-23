#pragma once

#include <cstdint>

#include "src/DebugToolDrivers/Protocols/RiscVDebugSpec/DebugModule/DebugModule.hpp"

namespace DebugToolDrivers::Protocols::RiscVDebugSpec::DebugModule::Registers
{
    struct StatusRegister
    {
        std::uint8_t version:4 = 0;
        bool validConfigStructurePointer:1 = false;
        bool supportsResetHalt:1 = false;
        bool authBusy:1 = false;
        bool authenticated:1 = false;
        bool anyHalted:1 = false;
        bool allHalted:1 = false;
        bool anyRunning:1 = false;
        bool allRunning:1 = false;
        bool anyUnavailable:1 = false;
        bool allUnavailable:1 = false;
        bool anyNonExistent:1 = false;
        bool allNonExistent:1 = false;
        bool anyResumeAcknowledge:1 = false;
        bool allResumeAcknowledge:1 = false;
        bool anyHaveReset:1 = false;
        bool allHaveReset:1 = false;
        bool implicitBreak:1 = false;
        bool stickyUnavailableBits:1 = false;
        bool ndmResetPending:1 = false;

        constexpr explicit StatusRegister(RegisterValue registerValue)
            : version(static_cast<std::uint8_t>(registerValue & 0x0F))
            , validConfigStructurePointer(static_cast<bool>(registerValue & (0x01 << 4)))
            , supportsResetHalt(static_cast<bool>(registerValue & (0x01 << 5)))
            , authBusy(static_cast<bool>(registerValue & (0x01 << 6)))
            , authenticated(static_cast<bool>(registerValue & (0x01 << 7)))
            , anyHalted(static_cast<bool>(registerValue & (0x01 << 8)))
            , allHalted(static_cast<bool>(registerValue & (0x01 << 9)))
            , anyRunning(static_cast<bool>(registerValue & (0x01 << 10)))
            , allRunning(static_cast<bool>(registerValue & (0x01 << 11)))
            , anyUnavailable(static_cast<bool>(registerValue & (0x01 << 12)))
            , allUnavailable(static_cast<bool>(registerValue & (0x01 << 13)))
            , anyNonExistent(static_cast<bool>(registerValue & (0x01 << 14)))
            , allNonExistent(static_cast<bool>(registerValue & (0x01 << 15)))
            , anyResumeAcknowledge(static_cast<bool>(registerValue & (0x01 << 16)))
            , allResumeAcknowledge(static_cast<bool>(registerValue & (0x01 << 17)))
            , anyHaveReset(static_cast<bool>(registerValue & (0x01 << 18)))
            , allHaveReset(static_cast<bool>(registerValue & (0x01 << 19)))
            , implicitBreak(static_cast<bool>(registerValue & (0x01 << 22)))
            , stickyUnavailableBits(static_cast<bool>(registerValue & (0x01 << 23)))
            , ndmResetPending(static_cast<bool>(registerValue & (0x01 << 24)))
        {}

        [[nodiscard]] constexpr RegisterValue value() const {
            return RegisterValue{0}
                | static_cast<RegisterValue>(this->version)
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
