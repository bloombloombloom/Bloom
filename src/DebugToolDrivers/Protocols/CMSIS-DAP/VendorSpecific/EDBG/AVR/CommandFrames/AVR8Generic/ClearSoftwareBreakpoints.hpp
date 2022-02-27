#pragma once

#include <cstdint>
#include <utility>

#include "Avr8GenericCommandFrame.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::Avr8Generic
{
    class ClearSoftwareBreakpoints: public Avr8GenericCommandFrame<std::vector<unsigned char>>
    {
    public:
        explicit ClearSoftwareBreakpoints(const std::vector<std::uint32_t>& addresses) {
            /*
             * The clear software breakpoints command consists of 2 bytes + 4*n bytes, where n is the number
             * of breakpoints to clear:
             *
             * 1. Command ID (0x44)
             * 2. Version (0x00)
             * ... addresses
             */
            this->payload = {
                0x44,
                0x00,
            };

            for (const auto& address : addresses) {
                this->payload.emplace_back(static_cast<unsigned char>(address));
                this->payload.emplace_back(static_cast<unsigned char>(address >> 8));
                this->payload.emplace_back(static_cast<unsigned char>(address >> 16));
                this->payload.emplace_back(static_cast<unsigned char>(address >> 24));
            }
        }
    };
}
