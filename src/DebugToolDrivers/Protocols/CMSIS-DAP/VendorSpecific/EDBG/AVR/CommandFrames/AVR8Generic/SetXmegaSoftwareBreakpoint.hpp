#pragma once

#include <cstdint>

#include "Avr8GenericCommandFrame.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::Avr8Generic
{
    class SetXmegaSoftwareBreakpoint: public Avr8GenericCommandFrame
    {
    private:
        std::uint32_t address;

    public:
        SetXmegaSoftwareBreakpoint() = default;

        SetXmegaSoftwareBreakpoint(std::uint32_t address) : address(address) {}

        void setAddress(std::uint32_t address) {
            this->address = address;
        }

        virtual std::vector<unsigned char> getPayload() const override {
            /*
             * The set software breakpoint command consists of 6 bytes bytes
             *
             * 1. Command ID (0x42)
             * 2. Version (0x00)
             * 3. Address (4 bytes)
             */
            auto output = std::vector<unsigned char>(15, 0x00);
            output[0] = 0x42;
            output[1] = 0x00;
            output[2] = static_cast<unsigned char>(address);
            output[3] = static_cast<unsigned char>(address >> 8);
            output[4] = static_cast<unsigned char>(address >> 16);
            output[5] = static_cast<unsigned char>(address >> 24);
            output[13] = 0x01; // One breakpoint
            output[14] = 0x00;

            return output;
        }
    };

}
