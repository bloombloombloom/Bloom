#pragma once

#include <cstdint>

#include "AvrIspCommandFrame.hpp"

#include "src/Targets/Microchip/AVR/Fuse.hpp"

namespace DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::AvrIsp
{
    class ReadFuse: public AvrIspCommandFrame<std::array<unsigned char, 6>>
    {
    public:
        ReadFuse(
            Targets::Microchip::Avr::FuseType fuseType,
            std::uint8_t returnAddress
        )
            : AvrIspCommandFrame()
        {
            using Targets::Microchip::Avr::FuseType;

            /*
             * The read fuse command consists of 6 bytes:
             *
             * 1. Command ID (0x18)
             * 2. Return Address
             * 3. CMD1
             * 4. CMD2
             * 5. CMD3
             * 6. CMD4
             */
            this->payload = {
                0x18,
                returnAddress,
                0x00,
                0x00,
                0x00,
                0x00,
            };

            switch (fuseType) {
                case FuseType::LOW: {
                    this->payload[2] = 0x50;
                    this->payload[3] = 0x00;
                    this->payload[4] = 0x00;
                    this->payload[5] = 0x00;
                    break;
                }
                case FuseType::HIGH: {
                    this->payload[2] = 0x58;
                    this->payload[3] = 0x08;
                    this->payload[4] = 0x00;
                    this->payload[5] = 0x00;
                    break;
                }
                case FuseType::EXTENDED: {
                    this->payload[2] = 0x50;
                    this->payload[3] = 0x08;
                    this->payload[4] = 0x00;
                    this->payload[5] = 0x00;
                    break;
                }
            }
        }
    };
}
