#pragma once

#include "src/DebugToolDrivers/Microchip/Protocols/EDBG/AVR/CommandFrames/AvrCommandFrame.hpp"
#include "src/DebugToolDrivers/Microchip/Protocols/EDBG/AVR/ResponseFrames/EDBGControl/EdbgControlResponseFrame.hpp"

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
