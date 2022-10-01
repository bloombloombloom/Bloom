#pragma once

#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AvrCommandFrame.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/ResponseFrames/HouseKeeping/HouseKeepingResponseFrame.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::HouseKeeping
{
    template<class PayloadContainerType>
    class HouseKeepingCommandFrame: public AvrCommandFrame<PayloadContainerType>
    {
    public:
        using ExpectedResponseFrameType = ResponseFrames::HouseKeeping::HouseKeepingResponseFrame;

        HouseKeepingCommandFrame()
            : AvrCommandFrame<PayloadContainerType>(ProtocolHandlerId::HOUSE_KEEPING)
        {}
    };
}
