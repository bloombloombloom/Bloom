#pragma once

#include <cstdint>

#include "Avr8GenericResponseFrame.hpp"

#include "src/Targets/TargetMemory.hpp"

#include "src/Exceptions/Exception.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::ResponseFrames::Avr8Generic
{
    class GetProgramCounter: public Avr8GenericResponseFrame
    {
    public:
        GetProgramCounter() = default;
        explicit GetProgramCounter(const std::vector<AvrResponse>& avrResponses)
            : Avr8GenericResponseFrame(avrResponses)
        {}

        Targets::TargetProgramCounter extractProgramCounter() {
            /*
             * The payload for the PC Read command should always consist of six bytes. Thr first two being the
             * command ID and version, the other four being the PC. The four PC bytes are little-endian.
             */
            auto& payload = this->getPayload();
            if (payload.size() != 6) {
                throw Exceptions::Exception("Failed to extract PC from payload of PC read command response "
                    "frame - unexpected payload size.");
            }

            return static_cast<Targets::TargetProgramCounter>(
                payload[5] << 24 | payload[4] << 16 | payload[3] << 8 | payload[2]
            ) * 2;
        }
    };
}
