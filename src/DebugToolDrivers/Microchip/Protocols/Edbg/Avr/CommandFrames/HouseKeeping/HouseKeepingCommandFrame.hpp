#pragma once

#include "src/DebugToolDrivers/Microchip/Protocols/Edbg/Avr/CommandFrames/AvrCommandFrame.hpp"
#include "src/DebugToolDrivers/Microchip/Protocols/Edbg/Avr/ResponseFrames/HouseKeeping/HouseKeepingResponseFrame.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr::CommandFrames::HouseKeeping
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
