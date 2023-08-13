#pragma once

#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/ResponseFrames/AvrResponseFrame.hpp"

namespace DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::ResponseFrames::EdbgControl
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
