#pragma once

#include <cstdint>

#include "../../AvrEvent.hpp"
#include "src/Targets/Microchip/AVR/Target.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr
{
    class BreakEvent: public AvrEvent
    {
    public:
        explicit BreakEvent(const AvrEvent& event) {
            this->init(event);
        }

        [[nodiscard]] std::uint32_t getProgramCounter() const {
            return this->programCounter;
        }

        [[nodiscard]] Targets::TargetBreakCause getBreakCause() const {
            return this->breakCause;
        }

    private:
        std::uint32_t programCounter = 0;
        Targets::TargetBreakCause breakCause = Targets::TargetBreakCause::UNKNOWN;

        void init(const AvrEvent& event);
    };
}
