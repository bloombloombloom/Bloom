#pragma once

#include <cstdint>

#include "EdbgControlCommandFrame.hpp"

namespace DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::EdbgControl
{
    class GetParameter: public EdbgControlCommandFrame<std::array<unsigned char, 5>>
    {
    public:
        GetParameter(const EdbgParameter& parameter)
            : EdbgControlCommandFrame()
        {
            /*
             * The EDBG Get Parameter command consists of 5 bytes:
             *
             * 1. Command ID (0x02)
             * 2. Version (0x00)
             * 3. Parameter context
             * 4. Parameter ID
             * 5. Parameter size
             */
            this->payload = {
                0x02,
                0x00,
                parameter.context,
                parameter.id,
                parameter.size,
            };
        }
    };
}
