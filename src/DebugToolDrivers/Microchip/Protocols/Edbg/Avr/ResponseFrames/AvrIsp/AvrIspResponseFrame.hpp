#pragma once

#include "src/DebugToolDrivers/Microchip/Protocols/Edbg/Avr/ResponseFrames/AvrResponseFrame.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr::ResponseFrames::AvrIsp
{
    enum class StatusCode: unsigned char
    {
        OK = 0x00,
        TIMEOUT = 0x80,
        FAILED = 0xC0,
    };

    class AvrIspResponseFrame: public AvrResponseFrame
    {
    public:
        StatusCode statusCode;

        explicit AvrIspResponseFrame(const std::vector<AvrResponse>& avrResponses);
    };
}
