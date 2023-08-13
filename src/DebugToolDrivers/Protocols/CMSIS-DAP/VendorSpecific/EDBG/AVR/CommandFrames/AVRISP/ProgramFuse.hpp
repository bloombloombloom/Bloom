#pragma once

#include <cstdint>

#include "AvrIspCommandFrame.hpp"

#include "src/Targets/Microchip/AVR/Fuse.hpp"

namespace DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::AvrIsp
{
    class ProgramFuse: public AvrIspCommandFrame<std::array<unsigned char, 5>>
    {
    public:
        ProgramFuse(Targets::Microchip::Avr::FuseType fuseType, unsigned char value)
            : AvrIspCommandFrame()
        {
            using Targets::Microchip::Avr::FuseType;

            /*
             * The program fuse command consists of 5 bytes:
             *
             * 1. Command ID (0x17)
             * 2. Return Address
             * 3. CMD1
             * 4. CMD2
             * 5. CMD3
             * 6. CMD4
             */
            this->payload = {
                0x17,
                0x00,
                0x00,
                0x00,
                value,
            };

            switch (fuseType) {
                case FuseType::LOW: {
                    this->payload[1] = 0xAC;
                    this->payload[2] = 0xA0;
                    this->payload[3] = 0x00;
                    break;
                }
                case FuseType::HIGH: {
                    this->payload[1] = 0xAC;
                    this->payload[2] = 0xA8;
                    this->payload[3] = 0x00;
                    break;
                }
                case FuseType::EXTENDED: {
                    this->payload[1] = 0xAC;
                    this->payload[2] = 0xA4;
                    this->payload[3] = 0x00;
                    break;
                }
                default: {
                    throw Exceptions::Exception("Unsupported fuse type");
                }
            }
        }
    };
}
