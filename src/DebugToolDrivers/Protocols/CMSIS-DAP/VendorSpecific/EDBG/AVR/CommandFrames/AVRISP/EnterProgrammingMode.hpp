#pragma once

#include <cstdint>

#include "AvrIspCommandFrame.hpp"

namespace DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::AvrIsp
{
    class EnterProgrammingMode: public AvrIspCommandFrame<std::array<unsigned char, 12>>
    {
    public:
        EnterProgrammingMode(
            std::uint8_t timeout,
            std::uint8_t stabDelay,
            std::uint8_t commandExecutionDelay,
            std::uint8_t syncLoops,
            std::uint8_t byteDelay,
            std::uint8_t pollValue,
            std::uint8_t pollIndex
        )
            : AvrIspCommandFrame()
        {
            /*
             * The enter programming mode command consists of 12 bytes:
             *
             * 1. Command ID (0x10)
             *
             * -- The fields from this point are taken from the AVR8 TDFs --
             *
             * 2. Command timeout in milliseconds
             * 3. Pin stabilization delay in milliseconds
             * 4. Command execution delay in milliseconds
             * 5. Number of synchronisation loops
             * 6. Millisecond delay between each byte in the command
             * 7. Poll value
             * 8. Poll index
             *
             * -- The fields from this point are the four bytes of the low-level SPI command --
             *
             * 9. CMD1 - For the enter programming mode command, this appears to always be 0xAC
             * 10. CMD2 - For AVR8 targets, this also appears to be a fixed value of 0x53
             * 11. CMD3 - 0x00
             * 12. CMD4 - 0x00
             */
            this->payload = {
                0x10,
                timeout,
                stabDelay,
                commandExecutionDelay,
                syncLoops,
                byteDelay,
                pollValue,
                pollIndex,
                0xAC,
                0x53,
                0x00,
                0x00,
            };
        }
    };
}
