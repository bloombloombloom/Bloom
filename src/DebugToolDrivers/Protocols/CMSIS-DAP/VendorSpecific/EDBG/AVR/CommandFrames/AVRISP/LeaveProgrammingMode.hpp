#pragma once

#include <cstdint>

#include "AvrIspCommandFrame.hpp"

namespace DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::AvrIsp
{
    class LeaveProgrammingMode: public AvrIspCommandFrame<std::array<unsigned char, 3>>
    {
    public:
        LeaveProgrammingMode(
            std::uint8_t preDelay,
            std::uint8_t postDelay
        )
            : AvrIspCommandFrame()
        {
            /*
             * The leave programming mode command consists of 3 bytes:
             *
             * 1. Command ID (0x11)
             *
             * -- The fields from this point are taken from the AVR8 TDFs --
             *
             * 2. Pre-delay
             * 3. Post-delay
             */
            this->payload = {
                0x11,
                preDelay,
                postDelay,
            };
        }
    };
}
