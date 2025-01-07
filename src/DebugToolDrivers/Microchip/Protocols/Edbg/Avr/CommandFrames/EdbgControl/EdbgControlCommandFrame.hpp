#pragma once

#include "src/DebugToolDrivers/Microchip/Protocols/Edbg/Avr/CommandFrames/AvrCommandFrame.hpp"
#include "src/DebugToolDrivers/Microchip/Protocols/Edbg/Avr/ResponseFrames/EdbgControl/EdbgControlResponseFrame.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr::CommandFrames::EdbgControl
{
    template<class PayloadContainerType>
    class EdbgControlCommandFrame: public AvrCommandFrame<PayloadContainerType>
    {
    public:
        using ExpectedResponseFrameType = ResponseFrames::EdbgControl::EdbgControlResponseFrame;

        EdbgControlCommandFrame()
            : AvrCommandFrame<PayloadContainerType>(ProtocolHandlerId::EDBG_CONTROL)
        {}
    };
}
