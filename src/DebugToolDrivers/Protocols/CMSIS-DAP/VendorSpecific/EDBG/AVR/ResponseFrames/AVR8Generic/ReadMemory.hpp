#pragma once

#include "Avr8GenericResponseFrame.hpp"
#include "src/Targets/TargetMemory.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::ResponseFrames::Avr8Generic
{
    class ReadMemory: public Avr8GenericResponseFrame
    {
    public:
        explicit ReadMemory(const std::vector<AvrResponse>& avrResponses)
            : Avr8GenericResponseFrame(avrResponses)
        {}

        Targets::TargetMemoryBuffer getMemoryData() const {
            /*
             * AVR8 data payloads are typically in little endian form, but this does not apply to the data returned
             * from the READ MEMORY commands.
             */
            return std::vector<unsigned char>(
                this->payload.begin() + 2,
                this->payload.end() - 1
            );
        }
    };
}
