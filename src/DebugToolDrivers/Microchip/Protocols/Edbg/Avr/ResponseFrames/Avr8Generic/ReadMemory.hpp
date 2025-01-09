#pragma once

#include "Avr8GenericResponseFrame.hpp"
#include "src/Targets/TargetMemory.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr::ResponseFrames::Avr8Generic
{
    class ReadMemory: public Avr8GenericResponseFrame
    {
    public:
        explicit ReadMemory(const std::vector<AvrResponse>& avrResponses)
            : Avr8GenericResponseFrame(avrResponses)
        {}

        [[nodiscard]] Targets::TargetMemoryBuffer getMemoryData() const {
            return {this->payload.begin() + 2, this->payload.end() - 1};
        }
    };
}
