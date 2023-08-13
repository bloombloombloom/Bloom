#pragma once

#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AvrCommandFrame.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/ResponseFrames/EDBGControl/EdbgControlResponseFrame.hpp"

namespace DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::EdbgControl
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
