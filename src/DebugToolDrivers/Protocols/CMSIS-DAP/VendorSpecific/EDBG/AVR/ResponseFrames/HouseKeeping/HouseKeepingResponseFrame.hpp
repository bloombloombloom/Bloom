#pragma once

#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/ResponseFrames/AvrResponseFrame.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::ResponseFrames::HouseKeeping
{
    enum class ResponseId: unsigned char
    {
        OK = 0x80,
        LIST = 0x81,
        DATA = 0x84,
        FAILED = 0xA0,
        FAILED_WITH_DATA = 0xA1,
    };

    class HouseKeepingResponseFrame: public AvrResponseFrame
    {
    public:
        ResponseId id;

        explicit HouseKeepingResponseFrame(const std::vector<AvrResponse>& avrResponses);
    };
}
