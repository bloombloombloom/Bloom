#pragma once

#include "src/Exceptions/Exception.hpp"
#include "src/DebugToolDrivers/Microchip/Protocols/Edbg/Avr/Avr8Generic.hpp"
#include "src/DebugToolDrivers/Microchip/Protocols/Edbg/Avr/CommandFrames/AvrCommandFrame.hpp"
#include "src/DebugToolDrivers/Microchip/Protocols/Edbg/Avr/ResponseFrames/Avr8Generic/Avr8GenericResponseFrame.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr::CommandFrames::Avr8Generic
{
    template<class PayloadContainerType>
    class Avr8GenericCommandFrame: public AvrCommandFrame<PayloadContainerType>
    {
    public:
        using ExpectedResponseFrameType = ResponseFrames::Avr8Generic::Avr8GenericResponseFrame;

        Avr8GenericCommandFrame()
            : AvrCommandFrame<PayloadContainerType>(ProtocolHandlerId::AVR8_GENERIC)
        {}
    };
}
