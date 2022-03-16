#pragma once

#include <cstdint>

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg
{
    enum class ProtocolHandlerId: unsigned char
    {
        DISCOVERY = 0x00,
        HOUSE_KEEPING = 0x01,
        AVRISP = 0x11,
        AVR8_GENERIC = 0x12,
        AVR32_GENERIC = 0x13,
        EDBG_CONTROL = 0x20,
    };

    struct EdbgParameter
    {
        unsigned char context = 0x00;
        unsigned char id = 0x00;
        std::uint8_t size = 0;

        constexpr EdbgParameter() = default;
        constexpr EdbgParameter(unsigned char context, unsigned char id, std::uint8_t size)
            : context(context), id(id), size(size) {};
    };

    struct EdbgParameters
    {
        static constexpr EdbgParameter CONTROL_LED_USAGE {0x00, 0x00, 1};
        static constexpr EdbgParameter CONTROL_TARGET_POWER {0x00, 0x10, 1};
    };
}
