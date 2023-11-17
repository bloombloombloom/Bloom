#pragma once

#include "src/DebugToolDrivers/Microchip/Protocols/EDBG/AVR/ResponseFrames/AvrResponseFrame.hpp"
#include "src/DebugToolDrivers/Microchip/Protocols/EDBG/AVR/Avr8Generic.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr::ResponseFrames::Avr8Generic
{
    class Avr8GenericResponseFrame: public AvrResponseFrame
    {
    public:
        Avr8ResponseId id;

        explicit Avr8GenericResponseFrame(const std::vector<AvrResponse>& avrResponses);

        [[nodiscard]] std::vector<unsigned char> getPayloadData() const;
    };
}
