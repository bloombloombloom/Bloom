#pragma once

#include <cstdint>

#include "src/DebugToolDrivers/Microchip/Protocols/Edbg/Avr/AvrEvent.hpp"
#include "src/Targets/TargetBreakpoint.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr
{
    class BreakEvent: public AvrEvent
    {
    public:
        std::uint32_t programCounter;
        Targets::TargetBreakCause breakCause;

        explicit BreakEvent(const AvrEvent& event);
    };
}
