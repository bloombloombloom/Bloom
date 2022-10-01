#pragma once

#include <cstdint>
#include <cassert>

#include "EdbgControlCommandFrame.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::EdbgControl
{
    class SetParameter: public EdbgControlCommandFrame<std::array<unsigned char, 6>>
    {
    public:
        SetParameter(const EdbgParameter& parameter, unsigned char value)
            : EdbgControlCommandFrame()
        {
            assert(parameter.size == 1);

            /*
             * The EDBG Set Parameter command consists of 6 bytes:
             *
             * 1. Command ID (0x01)
             * 2. Version (0x00)
             * 3. Parameter context
             * 4. Parameter
             * 5. Parameter size
             * 6. Parameter value
             */
            this->payload = {
                0x01,
                0x00,
                parameter.context,
                parameter.id,
                parameter.size,
                value,
            };
        }
    };
}
