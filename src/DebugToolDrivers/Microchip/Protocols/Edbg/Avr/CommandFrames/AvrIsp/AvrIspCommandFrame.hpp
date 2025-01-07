#pragma once

#include "src/Exceptions/Exception.hpp"
#include "src/DebugToolDrivers/Microchip/Protocols/Edbg/Avr/Avr8Generic.hpp"
#include "src/DebugToolDrivers/Microchip/Protocols/Edbg/Avr/CommandFrames/AvrCommandFrame.hpp"
#include "src/DebugToolDrivers/Microchip/Protocols/Edbg/Avr/ResponseFrames/AvrIsp/AvrIspResponseFrame.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr::CommandFrames::AvrIsp
{
    template<class PayloadContainerType>
    class AvrIspCommandFrame: public AvrCommandFrame<PayloadContainerType>
    {
    public:
        using ExpectedResponseFrameType = ResponseFrames::AvrIsp::AvrIspResponseFrame;

        AvrIspCommandFrame()
            : AvrCommandFrame<PayloadContainerType>(ProtocolHandlerId::AVRISP)
        {}
    };
}
