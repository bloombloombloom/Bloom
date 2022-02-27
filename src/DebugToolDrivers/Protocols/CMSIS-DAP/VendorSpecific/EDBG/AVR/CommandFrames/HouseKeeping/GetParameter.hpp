#pragma once

#include <cstdint>

#include "HouseKeepingCommandFrame.hpp"
#include "Parameters.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::HouseKeeping
{
    class GetParameter: public HouseKeepingCommandFrame<std::array<unsigned char, 5>>
    {
    public:
        explicit GetParameter(const Parameter& parameter, std::uint8_t size) {
            /*
             * The get param command consists of 5 bytes:
             * 1. Command ID (0x02)
             * 2. Version (0x00)
             * 3. Param context (Parameter::context)
             * 4. Param ID (Parameter::id)
             * 5. Param value length (this->size)
             */
            this->payload = {
                0x02,
                0x00,
                static_cast<unsigned char>(parameter.context),
                static_cast<unsigned char>(parameter.id),
                static_cast<unsigned char>(size)
            };
        }
    };
}
