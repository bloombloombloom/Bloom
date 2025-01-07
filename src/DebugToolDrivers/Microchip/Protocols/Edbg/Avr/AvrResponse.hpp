#pragma once

#include <cstdint>
#include <vector>

#include "src/DebugToolDrivers/Protocols/CmsisDap/Response.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr
{
    class AvrResponse: public ::DebugToolDrivers::Protocols::CmsisDap::Response
    {
    public:
        std::uint8_t fragmentNumber = 0;
        std::uint8_t fragmentCount = 0;

        std::vector<unsigned char> responsePacket;

        explicit AvrResponse(const std::vector<unsigned char>& rawResponse);
    };
}
