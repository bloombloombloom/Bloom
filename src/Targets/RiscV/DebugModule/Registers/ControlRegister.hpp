#pragma once

#include <cstdint>

#include "src/Targets/RiscV/DebugModule/DebugModule.hpp"

namespace Targets::RiscV::DebugModule::Registers
{
    struct ControlRegister
    {
        enum HartSelectionMode: std::uint8_t
        {
            SINGLE = 0x00,
            MULTI = 0x01,
        };

        bool debugModuleActive:1 = false;
        bool ndmReset:1 = false;
        bool clearResetHaltRequest:1 = false;
        bool setResetHaltRequest:1 = false;
        bool clearKeepAlive:1 = false;
        bool setKeepAlive:1 = false;
        HartIndex selectedHartIndex:20 = 0;
        HartSelectionMode hartSelectionMode:1 = HartSelectionMode::SINGLE;
        bool acknowledgeUnavailableHarts:1 = false;
        bool acknowledgeHaveReset:1 = false;
        bool hartReset:1 = false;
        bool resumeRequest:1 = false;
        bool haltRequest:1 = false;

        ControlRegister() = default;

        constexpr explicit ControlRegister(RegisterValue registerValue)
            : debugModuleActive(static_cast<bool>(registerValue & 0x01))
            , ndmReset(static_cast<bool>(registerValue & (0x01 << 1)))
            , clearResetHaltRequest(static_cast<bool>(registerValue & (0x01 << 2)))
            , setResetHaltRequest(static_cast<bool>(registerValue & (0x01 << 3)))
            , clearKeepAlive(static_cast<bool>(registerValue & (0x01 << 4)))
            , setKeepAlive(static_cast<bool>(registerValue & (0x01 << 5)))
            , selectedHartIndex((((registerValue >> 6) & 0x3FF) << 10) | ((registerValue >> 16) & 0x3FF))
            , hartSelectionMode(static_cast<HartSelectionMode>(registerValue & (0x01 << 26)))
            , acknowledgeUnavailableHarts(static_cast<bool>(registerValue & (0x01 << 27)))
            , acknowledgeHaveReset(static_cast<bool>(registerValue & (0x01 << 28)))
            , hartReset(static_cast<bool>(registerValue & (0x01 << 29)))
            , resumeRequest(static_cast<bool>(registerValue & (0x01 << 30)))
            , haltRequest(static_cast<bool>(registerValue & static_cast<std::uint32_t>(0x01 << 31)))
        {}

        constexpr RegisterValue value() const {
            return RegisterValue{0}
                | static_cast<RegisterValue>(this->debugModuleActive)
                | static_cast<RegisterValue>(this->ndmReset) << 1
                | static_cast<RegisterValue>(this->clearResetHaltRequest) << 2
                | static_cast<RegisterValue>(this->setResetHaltRequest) << 3
                | static_cast<RegisterValue>(this->clearKeepAlive) << 4
                | static_cast<RegisterValue>(this->setKeepAlive) << 5
                | static_cast<RegisterValue>(this->selectedHartIndex >> 10) << 6
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
