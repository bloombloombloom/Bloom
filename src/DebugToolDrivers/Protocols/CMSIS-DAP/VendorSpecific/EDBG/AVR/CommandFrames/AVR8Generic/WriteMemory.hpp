#pragma once

#include <cstdint>

#include "Avr8GenericCommandFrame.hpp"
#include "src/Targets/TargetMemory.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::Avr8Generic
{
    class WriteMemory: public Avr8GenericCommandFrame<std::vector<unsigned char>>
    {
    public:
        WriteMemory(const Avr8MemoryType& type, std::uint32_t address, const Targets::TargetMemoryBuffer& buffer)
            : Avr8GenericCommandFrame()
        {
            /*
             * The write memory command consists of 12 bytes + the buffer size:
             * 1. Command ID (0x23)
             * 2. Version (0x00)
             * 3. Memory type (Avr8MemoryType)
             * 4. Start address (4 bytes)
             * 5. Number of bytes to write (4 bytes)
             * 6. Asynchronous flag (0x00 for "write first, then reply" and 0x01 for "reply first, then write")
             * 7. Buffer
             */
            this->payload = std::vector<unsigned char>(12, 0x00);
            this->payload[0] = 0x23;
            this->payload[1] = 0x00;
            this->payload[2] = static_cast<unsigned char>(type);
            this->payload[3] = static_cast<unsigned char>(address);
            this->payload[4] = static_cast<unsigned char>(address >> 8);
            this->payload[5] = static_cast<unsigned char>(address >> 16);
            this->payload[6] = static_cast<unsigned char>(address >> 24);

            auto bytesToWrite = static_cast<std::uint32_t>(buffer.size());
            this->payload[7] = static_cast<unsigned char>(bytesToWrite);
            this->payload[8] = static_cast<unsigned char>(bytesToWrite >> 8);
            this->payload[9] = static_cast<unsigned char>(bytesToWrite >> 16);
            this->payload[10] = static_cast<unsigned char>(bytesToWrite >> 24);

            // We always set the async flag to 0x00 ("write first, then reply")
            this->payload[11] = 0x00;

            this->payload.insert(this->payload.end(), buffer.begin(), buffer.end());
        }
    };
}
