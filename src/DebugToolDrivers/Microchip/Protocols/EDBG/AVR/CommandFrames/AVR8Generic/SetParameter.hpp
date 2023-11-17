#pragma once

#include "Avr8GenericCommandFrame.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr::CommandFrames::Avr8Generic
{
    class SetParameter: public Avr8GenericCommandFrame<std::vector<unsigned char>>
    {
    public:
        SetParameter(const Avr8EdbgParameter& parameter, const std::vector<unsigned char>& value)
            : Avr8GenericCommandFrame()
        {
            /*
             * The set param command consists of value.size() + 5 bytes. The first five bytes consist of:
             * 1. Command ID (0x01)
             * 2. Version (0x00)
             * 3. Param context (Avr8EdbgParameter::context)
             * 4. Param ID (Avr8EdbgParameter::id)
             * 5. Param value length (value.size()) - this is only one byte in size, so its value should
             *    never exceed 255.
             */
            this->payload = std::vector<unsigned char>(5, 0x00);
            this->payload[0] = 0x01;
            this->payload[1] = 0x00;
            this->payload[2] = static_cast<unsigned char>(parameter.context);
            this->payload[3] = static_cast<unsigned char>(parameter.id);
            this->payload[4] = static_cast<unsigned char>(value.size());
            this->payload.insert(this->payload.end(), value.begin(), value.end());
        }
    };
}
