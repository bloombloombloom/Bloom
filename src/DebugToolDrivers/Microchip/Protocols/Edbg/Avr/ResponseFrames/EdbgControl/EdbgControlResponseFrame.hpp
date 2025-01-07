#pragma once

#include "src/DebugToolDrivers/Microchip/Protocols/Edbg/Avr/ResponseFrames/AvrResponseFrame.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr::ResponseFrames::EdbgControl
{
    enum class EdbgControlResponseId: unsigned char
    {
        OK = 0x80,
        DATA = 0x84,
        FAILED = 0xA0,
    };

    class EdbgControlResponseFrame: public AvrResponseFrame
    {
    public:
        EdbgControlResponseId id;

        explicit EdbgControlResponseFrame(const std::vector<AvrResponse>& avrResponses);

        [[nodiscard]] std::vector<unsigned char> getPayloadData();
    };
}
