#pragma once

#include <optional>
#include <vector>

#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/Response.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr
{
    enum class AvrEventId: unsigned char
    {
        AVR8_BREAK_EVENT = 0x40,
    };

    class AvrEvent: public ::DebugToolDrivers::Protocols::CmsisDap::Response
    {
    public:
        std::optional<AvrEventId> eventId = std::nullopt;
        std::vector<unsigned char> eventData = {};

        explicit AvrEvent(const std::vector<unsigned char>& rawResponse);
    };
}
