#pragma once

#include <cstdint>

#include "AvrIspCommandFrame.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::AvrIsp
{
    class ReadLock: public AvrIspCommandFrame<std::array<unsigned char, 6>>
    {
    public:
        explicit ReadLock(std::uint8_t returnAddress) {
            /*
             * The read lock command consists of 6 bytes:
             *
             * 1. Command ID (0x1A)
             * 2. Return Address
             * 3. CMD1
             * 4. CMD2
             * 5. CMD3
             * 6. CMD4
             */
            this->payload = {
                0x1A,
                returnAddress,
                0x58,
                0x00,
                0x00,
                0x00,
            };
        }
    };
}
