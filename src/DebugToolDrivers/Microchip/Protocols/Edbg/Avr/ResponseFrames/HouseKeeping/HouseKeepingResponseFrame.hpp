#pragma once

#include <cstdint>

#include "src/DebugToolDrivers/Microchip/Protocols/Edbg/Avr/ResponseFrames/AvrResponseFrame.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr::ResponseFrames::HouseKeeping
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
        std::uint8_t version;

        explicit HouseKeepingResponseFrame(const std::vector<AvrResponse>& avrResponses);

        [[nodiscard]] std::vector<unsigned char> getPayloadData() const;
    };
}
