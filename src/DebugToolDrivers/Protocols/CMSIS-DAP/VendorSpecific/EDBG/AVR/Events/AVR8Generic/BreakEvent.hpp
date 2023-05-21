#pragma once

#include <cstdint>

#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/AvrEvent.hpp"
#include "src/Targets/TargetBreakpoint.hpp"

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
