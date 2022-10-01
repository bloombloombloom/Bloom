#pragma once

#include <cstdint>

#include "../../AvrEvent.hpp"
#include "src/Targets/Microchip/AVR/Target.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr
{
    class BreakEvent: public AvrEvent
    {
    public:
        std::uint32_t programCounter;
        Targets::TargetBreakCause breakCause;

        explicit BreakEvent(const AvrEvent& event);
    };
}
