#pragma once

#include <cstdint>

#include "../../AvrEvent.hpp"
#include "src/Targets/Microchip/AVR/Target.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr
{
    class BreakEvent: public AvrEvent
    {
    private:
        std::uint32_t programCounter;
        Targets::TargetBreakCause breakCause;

        void init(const AvrEvent& event);

    public:
        BreakEvent(const AvrEvent& event) {
            this->init(event);
        }

        std::uint32_t getProgramCounter() {
            return this->programCounter;
        }

        Targets::TargetBreakCause getBreakCause() {
            return this->breakCause;
        }
    };
}
