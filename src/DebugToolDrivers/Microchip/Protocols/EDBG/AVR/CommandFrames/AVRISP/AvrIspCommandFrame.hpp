#pragma once

#include "src/Exceptions/Exception.hpp"
#include "src/DebugToolDrivers/Microchip/Protocols/EDBG/AVR/Avr8Generic.hpp"
#include "src/DebugToolDrivers/Microchip/Protocols/EDBG/AVR/CommandFrames/AvrCommandFrame.hpp"
#include "src/DebugToolDrivers/Microchip/Protocols/EDBG/AVR/ResponseFrames/AVRISP/AvrIspResponseFrame.hpp"

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
