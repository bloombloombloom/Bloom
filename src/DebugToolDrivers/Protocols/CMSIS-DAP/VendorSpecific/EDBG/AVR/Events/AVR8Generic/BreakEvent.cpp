#include "BreakEvent.hpp"

#include "src/Exceptions/Exception.hpp"

namespace DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr
{
    using namespace Exceptions;

    using Targets::TargetBreakCause;

    BreakEvent::BreakEvent(const AvrEvent& event)
        : AvrEvent(event)
    {
        if (this->eventData.size() < 8) {
            /*
             * All BreakEvent packets must consist of at least 8 bytes:
             * 1 byte for event ID
             * 4 bytes for program counter
             * 1 byte for break cause
             * 2 bytes for extended info
             */
            throw Exception("Failed to process BreakEvent from AvrEvent - unexpected packet size.");
        }

        // Program counter consists of 4 bytes
        this->programCounter = static_cast<std::uint32_t>(
            (this->eventData[4] << 24) | (this->eventData[3] << 16) | (this->eventData[2] << 8) | this->eventData[1]
        ) * 2;

        // Break cause is 1 byte, where 0x01 is 'program breakpoint' and 0x00 'unspecified'
        this->breakCause = this->eventData[7] == 0x01 ? TargetBreakCause::BREAKPOINT : TargetBreakCause::UNKNOWN;
    }
}
