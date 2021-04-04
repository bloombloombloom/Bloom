#pragma once

#include <cstdint>

#include "Avr8GenericCommandFrame.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::Avr8Generic
{
    class SetSoftwareBreakpoints: public Avr8GenericCommandFrame
    {
    private:
        std::vector<std::uint32_t> addresses;

    public:
        SetSoftwareBreakpoints() = default;

        SetSoftwareBreakpoints(const std::vector<std::uint32_t>& addresses) : addresses(addresses) {}

        void setAddresses(const std::vector<std::uint32_t>& addresses) {
            this->addresses = addresses;
        }

        virtual std::vector<unsigned char> getPayload() const override {
            /*
             * The set software breakpoint command consists of 2 bytes + 4*n bytes, where n is the number
             * of breakpoints to set:
             *
             * 1. Command ID (0x43)
             * 2. Version (0x00)
             * ... addresses
             */
            auto output = std::vector<unsigned char>(2, 0x00);
            output[0] = 0x43;
            output[1] = 0x00;

            for (const auto& address : this->addresses) {
                output.push_back(static_cast<unsigned char>(address));
                output.push_back(static_cast<unsigned char>(address >> 8));
                output.push_back(static_cast<unsigned char>(address >> 16));
                output.push_back(static_cast<unsigned char>(address >> 24));
            }

            return output;
        }
    };

}
