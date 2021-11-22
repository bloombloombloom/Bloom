#pragma once

#include <cstdint>

#include "HouseKeepingCommandFrame.hpp"
#include "Parameters.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::HouseKeeping
{
    class GetParameter: public HouseKeepingCommandFrame
    {
    public:
        explicit GetParameter(const Parameter& parameter): parameter(parameter) {}

        GetParameter(const Parameter& parameter, std::uint8_t size): GetParameter(parameter) {
            this->setSize(size);
        }

        void setParameter(const Parameter& parameter) {
            this->parameter = parameter;
        }

        void setSize(std::uint8_t size) {
            this->size = size;
        }

        [[nodiscard]] std::vector<unsigned char> getPayload() const override {
            /*
             * The get param command consists of 5 bytes:
             * 1. Command ID (0x02)
             * 2. Version (0x00)
             * 3. Param context (Parameter::context)
             * 4. Param ID (Parameter::id)
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

    private:
        Parameter parameter;
        std::uint8_t size = 0;
    };
}
