#pragma once

#include "src/Exceptions/Exception.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/Avr8Generic.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AvrCommandFrame.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/ResponseFrames/AVR8Generic/Avr8GenericResponseFrame.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::Avr8Generic
{
    class Avr8GenericCommandFrame: public AvrCommandFrame
    {
    public:
        using ResponseFrameType = ResponseFrames::Avr8Generic::Avr8GenericResponseFrame;

        Avr8GenericCommandFrame() {
            this->setProtocolHandlerId(ProtocolHandlerId::AVR8_GENERIC);
        }
    };
}
