#pragma once

#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/ResponseFrames/AvrResponseFrame.hpp"

namespace DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::ResponseFrames::Discovery
{
    /**
     * Discovery commands can only return two responses; A LIST response and a failure.
     */
    enum class ResponseId: unsigned char
    {
        /*
         * According to the protocol docs, response ID 0x81 is for a LIST response, but this doesn't seem to be
         * well-defined. So just going to use a generic name.
         */
        OK = 0x81,
        FAILED = 0xA0,
    };

    class DiscoveryResponseFrame: public AvrResponseFrame
    {
    public:
        ResponseId id;

        explicit DiscoveryResponseFrame(const std::vector<AvrResponse>& avrResponses);

        std::vector<unsigned char> getPayloadData() const;
    };
}
