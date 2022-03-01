#pragma once

#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/ResponseFrames/AvrResponseFrame.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::ResponseFrames
{
    class HouseKeepingResponseFrame: public AvrResponseFrame
    {
    public:
        HouseKeepingResponseFrame() = default;
        explicit HouseKeepingResponseFrame(const std::vector<AvrResponse>& avrResponses): AvrResponseFrame(avrResponses) {}

        unsigned char getResponseId();
    };
}
