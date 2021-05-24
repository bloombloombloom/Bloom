#include <cstdint>

#include "BreakEvent.hpp"
#include "src/Exceptions/Exception.hpp"

using namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr;
using namespace Bloom::Targets;
using namespace Bloom::Exceptions;

void BreakEvent::init(const AvrEvent& event) {
    AvrEvent::init(event);
    auto& data = this->getEventData();

    if (data.size() < 8) {
        /*
         * All BreakEvent packets must consist of at least 9 bytes:
         * 1 byte for event ID
         * 4 bytes for program counter
         * 1 byte for break cause
         * 2 bytes for extended info
         */
        throw Exception("Failed to process BreakEvent from AvrEvent - unexpected packet size.");
    }

    // Program counter consists of 4 bytes
    this->programCounter = static_cast<std::uint32_t>((data[4] << 24) | (data[3] << 16) | (data[2] << 8) | data[1]) * 2;

    // Break cause is 1 byte, where 0x01 is 'program breakpoint' and 0x00 'unspecified'
    this->breakCause = data[7] == 0x01 ? TargetBreakCause::BREAKPOINT : TargetBreakCause::UNKNOWN;
}
