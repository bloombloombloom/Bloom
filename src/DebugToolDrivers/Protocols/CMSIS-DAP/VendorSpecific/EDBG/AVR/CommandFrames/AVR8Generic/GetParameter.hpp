#pragma once

#include <cstdint>

#include "Avr8GenericCommandFrame.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::Avr8Generic
{
    class GetParameter: public Avr8GenericCommandFrame
    {
    private:
        Avr8EdbgParameter parameter;
        std::uint8_t size;

    public:
        GetParameter() = default;

        GetParameter(const Avr8EdbgParameter& parameter) {
            this->setParameter(parameter);
        }

        GetParameter(const Avr8EdbgParameter& parameter, std::uint8_t size): GetParameter(parameter) {
            this->setSize(size);
        }

        void setParameter(const Avr8EdbgParameter& parameter) {
            this->parameter = parameter;
        }

        void setSize(std::uint8_t size) {
            this->size = size;
        }

        virtual std::vector<unsigned char> getPayload() const override {
            /*
             * The get param command consists of 5 bytes:
             * 1. Command ID (0x02)
             * 2. Version (0x00)
             * 3. Param context (Avr8Parameter::context)
             * 4. Param ID (Avr8Parameter::id)
             * 5. Param value length (this->size)
             */
            auto output = std::vector<unsigned char>(5, 0x00);
            output[0] = 0x02;
            output[1] = 0x00;
            output[2] = static_cast<unsigned char>(this->parameter.context);
            output[3] = static_cast<unsigned char>(this->parameter.id);
            output[4] = static_cast<unsigned char>(this->size);

            return output;
        }
    };

}
