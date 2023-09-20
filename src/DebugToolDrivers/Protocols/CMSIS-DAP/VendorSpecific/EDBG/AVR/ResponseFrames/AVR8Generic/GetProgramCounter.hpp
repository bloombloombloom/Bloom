#pragma once

#include <cstdint>

#include "Avr8GenericResponseFrame.hpp"

#include "src/Targets/TargetMemory.hpp"

#include "src/Exceptions/Exception.hpp"

namespace DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::ResponseFrames::Avr8Generic
{
    class GetProgramCounter: public Avr8GenericResponseFrame
    {
    public:
        explicit GetProgramCounter(const std::vector<AvrResponse>& avrResponses)
            : Avr8GenericResponseFrame(avrResponses)
        {}

        Targets::TargetMemoryAddress extractProgramCounter() const {
            /*
             * The payload for the PC Read command should always consist of six bytes. Thr first two being the
             * command ID and version, the other four being the PC. The four PC bytes are little-endian.
             */
            if (this->payload.size() != 6) {
                throw Exceptions::Exception("Failed to extract PC from payload of PC read command response "
                    "frame - unexpected payload size.");
            }

            return static_cast<Targets::TargetMemoryAddress>(
                this->payload[5] << 24 | this->payload[4] << 16 | this->payload[3] << 8 | this->payload[2]
            ) * 2;
        }
    };
}
