#pragma once

#include "Avr8GenericResponseFrame.hpp"
#include "src/Targets/TargetMemory.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::ResponseFrames::Avr8Generic
{
    class ReadMemory: public Avr8GenericResponseFrame
    {
    public:
        ReadMemory() = default;
        explicit ReadMemory(const std::vector<AvrResponse>& AVRResponses): Avr8GenericResponseFrame(AVRResponses) {}

        Targets::TargetMemoryBuffer getMemoryBuffer() {
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
