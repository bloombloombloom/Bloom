#pragma once

#include "Avr8GenericResponseFrame.hpp"
#include "src/Targets/TargetMemory.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::ResponseFrames::Avr8Generic
{
    using namespace Bloom::Exceptions;
    using Bloom::Targets::TargetMemoryBuffer;

    class ReadMemory: public Avr8GenericResponseFrame
    {
    public:
        ReadMemory(const std::vector<AvrResponse>& AVRResponses): Avr8GenericResponseFrame(AVRResponses) {}
        ReadMemory() {}

        TargetMemoryBuffer getMemoryBuffer() {
            /*
             * AVR8 data payloads are typically in little endian form, but this does not apply the data returned
             * from the READ MEMORY commands.
             */
            auto data = std::vector<unsigned char>(
                this->getPayload().begin() + 2,
                this->getPayload().end() - 1
            );

            return data;
        }
    };

}
