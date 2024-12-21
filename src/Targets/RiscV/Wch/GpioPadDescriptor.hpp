#pragma once

#include <cstdint>

#include "src/Targets/TargetPadDescriptor.hpp"
#include "src/Targets/TargetRegisterDescriptor.hpp"
#include "src/Targets/TargetBitFieldDescriptor.hpp"

namespace Targets::RiscV::Wch
{
    enum class GpioPadDirection: std::uint8_t
    {
        INPUT = 0x00,
        OUTPUT = 0x01,
    };

    enum class GpioPadInputMode: std::uint8_t
    {
        ANALOG = 0x00,
        FLOATING = 0x01,
        PULLED = 0x10,
    };

    enum class GpioPadOutputMode: std::uint8_t
    {
        GENERAL_PURPOSE = 0x00,
        ALTERNATE_FUNCTION = 0x01,
    };

    struct GpioPadDescriptor
    {
        const TargetBitFieldDescriptor& peripheralClockEnableBitFieldDescriptor;
        const TargetRegisterDescriptor& configRegisterDescriptor;
        const TargetBitFieldDescriptor& configBitFieldDescriptor;
        const TargetBitFieldDescriptor& modeBitFieldDescriptor;
        const TargetRegisterDescriptor& inputDataRegisterDescriptor;
        const TargetBitFieldDescriptor& inputDataBitFieldDescriptor;
        const TargetRegisterDescriptor& outputDataRegisterDescriptor;
        const TargetBitFieldDescriptor& outputDataBitFieldDescriptor;
    };
}
