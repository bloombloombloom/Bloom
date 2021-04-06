#pragma once

#include "Avr8GenericCommandFrame.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::Avr8Generic
{
    using namespace Exceptions;

    class SetParameter: public Avr8GenericCommandFrame
    {
    private:
        Avr8EdbgParameter parameter;
        std::vector<unsigned char> value;

    public:
        SetParameter() = default;

        SetParameter(const Avr8EdbgParameter& parameter) {
            this->setParameter(parameter);
        }

        SetParameter(const Avr8EdbgParameter& parameter, const std::vector<unsigned char>& value): SetParameter(parameter) {
            this->setValue(value);
        }

        SetParameter(const Avr8EdbgParameter& parameter, unsigned char value): SetParameter(parameter) {
            this->setValue(value);
        }

        void setParameter(const Avr8EdbgParameter& parameter) {
            this->parameter = parameter;
        }

        void setValue(const std::vector<unsigned char>& value) {
            this->value = value;
        }

        void setValue(unsigned char value) {
            this->value.resize(1, value);
        }

        virtual std::vector<unsigned char> getPayload() const override {
            /*
             * The set param command consists of this->value.size() + 5 bytes. The first five bytes consist of:
             * 1. Command ID (0x01)
             * 2. Version (0x00)
             * 3. Param context (Avr8Parameter::context)
             * 4. Param ID (Avr8Parameter::id)
             * 5. Param value length (this->value.size()) - this is only one byte in size, so its value should
             *    never exceed 255.
             */
            auto output = std::vector<unsigned char>(this->value.size() + 5, 0x00);
            output[0] = 0x01;
            output[1] = 0x00;
            output[2] = static_cast<unsigned char>(this->parameter.context);
            output[3] = static_cast<unsigned char>(this->parameter.id);
            output[4] = static_cast<unsigned char>(this->value.size());
            output.insert(output.begin() + 5, this->value.begin(), this->value.end());

            return output;
        }
    };

}
