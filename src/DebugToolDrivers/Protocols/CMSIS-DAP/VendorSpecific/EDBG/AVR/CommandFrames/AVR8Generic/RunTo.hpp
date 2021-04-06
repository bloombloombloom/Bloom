#pragma once

#include <cstdint>

#include "Avr8GenericCommandFrame.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::Avr8Generic
{
    class RunTo: public Avr8GenericCommandFrame
    {
    private:
        std::uint32_t address;

    public:
        RunTo() = default;

        RunTo(const std::uint32_t& address): address(address) {}

        void setAddress(const std::uint32_t& address) {
            this->address = address;
        }

        virtual std::vector<unsigned char> getPayload() const override {
            /*
             * The run-to command consists of 6 bytes:
             *
             * 1. Command ID (0x33)
             * 2. Version (0x00)
             * 3. Address to run to (4 bytes)
             */
            auto output = std::vector<unsigned char>(6, 0x00);
            output[0] = 0x33;
            output[1] = 0x00;

            // The address to run to needs to be the 16-bit word address, not the byte address.
            auto wordAddress = this->address / 2;
            output[2] = (static_cast<unsigned char>(wordAddress));
            output[3] = (static_cast<unsigned char>(wordAddress >> 8));
            output[4] = (static_cast<unsigned char>(wordAddress >> 16));
            output[5] = (static_cast<unsigned char>(wordAddress >> 24));

            return output;
        }
    };
}
