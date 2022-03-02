#pragma once

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg
{
    enum class ProtocolHandlerId: unsigned char
    {
        DISCOVERY = 0x00,
        HOUSE_KEEPING = 0x01,
        AVRISP = 0x11,
        AVR8_GENERIC = 0x12,
        AVR32_GENERIC = 0x13,
    };
}
