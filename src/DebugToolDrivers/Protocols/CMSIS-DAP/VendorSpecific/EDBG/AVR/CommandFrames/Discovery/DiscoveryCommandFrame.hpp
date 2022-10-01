#pragma once

#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AvrCommandFrame.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/ResponseFrames/Discovery/DiscoveryResponseFrame.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::Discovery
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
