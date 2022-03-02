#pragma once

#include "src/Exceptions/Exception.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/Avr8Generic.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AvrCommandFrame.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/ResponseFrames/AVRISP/AvrIspResponseFrame.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::AvrIsp
{
    template<class PayloadContainerType>
    class AvrIspCommandFrame: public AvrCommandFrame<PayloadContainerType>
    {
    public:
        using ExpectedResponseFrameType = ResponseFrames::AvrIsp::AvrIspResponseFrame;

        AvrIspCommandFrame(): AvrCommandFrame<PayloadContainerType>(ProtocolHandlerId::AVRISP) {}
    };
}
