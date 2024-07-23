#pragma once

#include <cstdint>
#include <src/Targets/Microchip/AVR8/Fuse.hpp>

#include "AvrIspCommandFrame.hpp"

#include "src/Targets/Microchip/AVR8/Fuse.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr::CommandFrames::AvrIsp
{
    class ReadSignature: public AvrIspCommandFrame<std::array<unsigned char, 6>>
    {
    public:
        ReadSignature(
            std::uint8_t signatureByteAddress,
            std::uint8_t returnAddress
        )
            : AvrIspCommandFrame()
        {
            using Targets::Microchip::Avr8::FuseType;

            /*
             * The read signature command consists of 6 bytes:
             *
             * 1. Command ID (0x1B)
             * 2. Return Address
             * 3. CMD1
             * 4. CMD2
             * 5. CMD3
             * 6. CMD4
             */
            this->payload = {
                0x1B,
                returnAddress,
                0x30,
                0x00,
                static_cast<unsigned char>(signatureByteAddress & 0x03),
                0x00,
            };
        }
    };
}
