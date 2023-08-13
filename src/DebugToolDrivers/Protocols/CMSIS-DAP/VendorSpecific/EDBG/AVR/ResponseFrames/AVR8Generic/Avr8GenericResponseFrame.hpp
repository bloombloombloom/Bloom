#pragma once

#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/ResponseFrames/AvrResponseFrame.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/Avr8Generic.hpp"

namespace DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::ResponseFrames::Avr8Generic
{
    class Avr8GenericResponseFrame: public AvrResponseFrame
    {
    public:
        Avr8ResponseId id;

        explicit Avr8GenericResponseFrame(const std::vector<AvrResponse>& avrResponses);

        [[nodiscard]] std::vector<unsigned char> getPayloadData() const;
    };
}
