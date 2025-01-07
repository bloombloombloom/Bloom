#pragma once

#include "Avr8GenericResponseFrame.hpp"

#include "src/Targets/Microchip/Avr8/TargetSignature.hpp"
#include "src/Targets/TargetPhysicalInterface.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr::ResponseFrames::Avr8Generic
{
    class GetDeviceId: public Avr8GenericResponseFrame
    {
    public:
        explicit GetDeviceId(const std::vector<AvrResponse>& AvrResponses);

        Targets::Microchip::Avr8::TargetSignature extractSignature(
            Targets::TargetPhysicalInterface physicalInterface
        ) const;
    };
}
