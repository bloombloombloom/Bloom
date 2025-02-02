#pragma once

#include <cstdint>

#include "src/DebugToolDrivers/Protocols/RiscVDebug/DebugModule/DebugModule.hpp"

namespace DebugToolDrivers::Protocols::RiscVDebug::DebugModule::Registers
{
    struct AbstractCommandAutoExecuteRegister
    {
        bool onData0Access = false;
        bool onData1Access = false;
        bool onData2Access = false;
        bool onData3Access = false;
        bool onData4Access = false;
        bool onData5Access = false;
        bool onData6Access = false;
        bool onData7Access = false;
        bool onData8Access = false;
        bool onData9Access = false;
        bool onData10Access = false;
        bool onData11Access = false;

        bool onProgramBuffer0Access = false;
        bool onProgramBuffer1Access = false;
        bool onProgramBuffer2Access = false;
        bool onProgramBuffer3Access = false;
        bool onProgramBuffer4Access = false;
        bool onProgramBuffer5Access = false;
        bool onProgramBuffer6Access = false;
        bool onProgramBuffer7Access = false;
        bool onProgramBuffer8Access = false;
        bool onProgramBuffer9Access = false;
        bool onProgramBuffer10Access = false;
        bool onProgramBuffer11Access = false;
        bool onProgramBuffer12Access = false;
        bool onProgramBuffer13Access = false;
        bool onProgramBuffer14Access = false;
        bool onProgramBuffer15Access = false;

        static constexpr AbstractCommandAutoExecuteRegister fromValue(RegisterValue value) {
            return {
                .onData0Access = static_cast<bool>(value & 0x01U),
                .onData1Access = static_cast<bool>(value & (0x01U << 1)),
                .onData2Access = static_cast<bool>(value & (0x01U << 2)),
                .onData3Access = static_cast<bool>(value & (0x01U << 3)),
                .onData4Access = static_cast<bool>(value & (0x01U << 4)),
                .onData5Access = static_cast<bool>(value & (0x01U << 5)),
                .onData6Access = static_cast<bool>(value & (0x01U << 6)),
                .onData7Access = static_cast<bool>(value & (0x01U << 7)),
                .onData8Access = static_cast<bool>(value & (0x01U << 8)),
                .onData9Access = static_cast<bool>(value & (0x01U << 9)),
                .onData10Access = static_cast<bool>(value & (0x01U << 10)),
                .onData11Access = static_cast<bool>(value & (0x01U << 11)),
                .onProgramBuffer0Access = static_cast<bool>(value & (0x01U << 16)),
                .onProgramBuffer1Access = static_cast<bool>(value & (0x01U << 17)),
                .onProgramBuffer2Access = static_cast<bool>(value & (0x01U << 18)),
                .onProgramBuffer3Access = static_cast<bool>(value & (0x01U << 19)),
                .onProgramBuffer4Access = static_cast<bool>(value & (0x01U << 20)),
                .onProgramBuffer5Access = static_cast<bool>(value & (0x01U << 21)),
                .onProgramBuffer6Access = static_cast<bool>(value & (0x01U << 22)),
                .onProgramBuffer7Access = static_cast<bool>(value & (0x01U << 23)),
                .onProgramBuffer8Access = static_cast<bool>(value & (0x01U << 24)),
                .onProgramBuffer9Access = static_cast<bool>(value & (0x01U << 25)),
                .onProgramBuffer10Access = static_cast<bool>(value & (0x01U << 26)),
                .onProgramBuffer11Access = static_cast<bool>(value & (0x01U << 27)),
                .onProgramBuffer12Access = static_cast<bool>(value & (0x01U << 28)),
                .onProgramBuffer13Access = static_cast<bool>(value & (0x01U << 29)),
                .onProgramBuffer14Access = static_cast<bool>(value & (0x01U << 30)),
                .onProgramBuffer15Access = static_cast<bool>(value & (0x01U << 31)),
            };
        }

        [[nodiscard]] constexpr RegisterValue value() const {
            return RegisterValue{0}
                | static_cast<RegisterValue>(this->onData0Access)
                | static_cast<RegisterValue>(this->onData1Access) << 1
                | static_cast<RegisterValue>(this->onData2Access) << 2
                | static_cast<RegisterValue>(this->onData3Access) << 3
                | static_cast<RegisterValue>(this->onData4Access) << 4
                | static_cast<RegisterValue>(this->onData5Access) << 5
                | static_cast<RegisterValue>(this->onData6Access) << 6
                | static_cast<RegisterValue>(this->onData7Access) << 7
                | static_cast<RegisterValue>(this->onData8Access) << 8
                | static_cast<RegisterValue>(this->onData9Access) << 9
                | static_cast<RegisterValue>(this->onData10Access) << 10
                | static_cast<RegisterValue>(this->onData11Access) << 11
                | static_cast<RegisterValue>(this->onProgramBuffer0Access) << 16
                | static_cast<RegisterValue>(this->onProgramBuffer1Access) << 17
                | static_cast<RegisterValue>(this->onProgramBuffer2Access) << 18
                | static_cast<RegisterValue>(this->onProgramBuffer3Access) << 19
                | static_cast<RegisterValue>(this->onProgramBuffer4Access) << 20
                | static_cast<RegisterValue>(this->onProgramBuffer5Access) << 21
                | static_cast<RegisterValue>(this->onProgramBuffer6Access) << 22
                | static_cast<RegisterValue>(this->onProgramBuffer7Access) << 23
                | static_cast<RegisterValue>(this->onProgramBuffer8Access) << 24
                | static_cast<RegisterValue>(this->onProgramBuffer9Access) << 25
                | static_cast<RegisterValue>(this->onProgramBuffer10Access) << 26
                | static_cast<RegisterValue>(this->onProgramBuffer11Access) << 27
                | static_cast<RegisterValue>(this->onProgramBuffer12Access) << 28
                | static_cast<RegisterValue>(this->onProgramBuffer13Access) << 29
                | static_cast<RegisterValue>(this->onProgramBuffer14Access) << 30
                | static_cast<RegisterValue>(this->onProgramBuffer15Access) << 31
            ;
        }
    };
}
