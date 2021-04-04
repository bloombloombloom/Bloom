#pragma once

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg
{
    enum class ProtocolHandlerId: unsigned char
    {
        Discovery = 0x00,
        HouseKeeping = 0x01,
        Avr8Generic = 0x12,
        Avr32Generic = 0x13,
    };
}