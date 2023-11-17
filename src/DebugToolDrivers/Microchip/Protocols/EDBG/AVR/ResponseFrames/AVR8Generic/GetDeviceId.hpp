#pragma once

#include "Avr8GenericResponseFrame.hpp"

#include "src/Targets/Microchip/AVR/TargetSignature.hpp"
#include "src/Targets/Microchip/AVR/AVR8/PhysicalInterface.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr::ResponseFrames::Avr8Generic
{
    class GetDeviceId: public Avr8GenericResponseFrame
    {
    public:
        explicit GetDeviceId(const std::vector<AvrResponse>& AvrResponses);

        Targets::Microchip::Avr::TargetSignature extractSignature(
            Targets::Microchip::Avr::Avr8Bit::PhysicalInterface physicalInterface
        ) const;
    };
}
