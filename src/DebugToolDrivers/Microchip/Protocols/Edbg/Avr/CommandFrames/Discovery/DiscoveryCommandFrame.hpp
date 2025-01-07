#pragma once

#include "src/DebugToolDrivers/Microchip/Protocols/Edbg/Avr/CommandFrames/AvrCommandFrame.hpp"
#include "src/DebugToolDrivers/Microchip/Protocols/Edbg/Avr/ResponseFrames/Discovery/DiscoveryResponseFrame.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr::CommandFrames::Discovery
{
    template<class PayloadContainerType>
    class DiscoveryCommandFrame: public AvrCommandFrame<PayloadContainerType>
    {
    public:
        using ExpectedResponseFrameType = ResponseFrames::Discovery::DiscoveryResponseFrame;

        DiscoveryCommandFrame()
            : AvrCommandFrame<PayloadContainerType>(ProtocolHandlerId::DISCOVERY)
        {}
    };
}
