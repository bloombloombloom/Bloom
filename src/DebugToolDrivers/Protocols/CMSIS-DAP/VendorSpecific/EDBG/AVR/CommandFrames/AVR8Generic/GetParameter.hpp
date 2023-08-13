#pragma once

#include <cstdint>

#include "Avr8GenericCommandFrame.hpp"

namespace DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::Avr8Generic
{
    class GetParameter: public Avr8GenericCommandFrame<std::array<unsigned char, 5>>
    {
    public:
        GetParameter(const Avr8EdbgParameter& parameter, std::uint8_t size)
            : Avr8GenericCommandFrame()
        {
            /*
             * The get param command consists of 5 bytes:
             * 1. Command ID (0x02)
             * 2. Version (0x00)
             * 3. Param context (Avr8Parameter::context)
             * 4. Param ID (Avr8Parameter::id)
             * 5. Param value length (size)
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
