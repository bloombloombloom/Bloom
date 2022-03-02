#pragma once

#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/ResponseFrames/AvrResponseFrame.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::ResponseFrames::AvrIsp
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
        AvrIspResponseFrame() = default;
        explicit AvrIspResponseFrame(const std::vector<AvrResponse>& avrResponses)
        : AvrResponseFrame(avrResponses) {}

        /**
         * Extracts the status code from the response frame payload.
         *
         * @return
         */
        [[nodiscard]] StatusCode getStatusCode();
    };
}
