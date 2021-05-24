#pragma once

#include <cstdint>

#include "Avr8GenericCommandFrame.hpp"
#include "src/Targets/TargetMemory.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::Avr8Generic
{
    class WriteMemory: public Avr8GenericCommandFrame
    {
    private:
        Avr8MemoryType type;
        std::uint32_t address = 0;
        Targets::TargetMemoryBuffer buffer;

    public:
        WriteMemory() = default;

        void setType(const Avr8MemoryType& type) {
            this->type = type;
        }

        void setAddress(std::uint32_t address) {
            this->address = address;
        }

        void setBuffer(const Targets::TargetMemoryBuffer& buffer) {
            this->buffer = buffer;
        }

        virtual std::vector<unsigned char> getPayload() const override {
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
            auto output = std::vector<unsigned char>(12, 0x00);
            output[0] = 0x23;
            output[1] = 0x00;
            output[2] = static_cast<unsigned char>(this->type);
            output[3] = static_cast<unsigned char>(this->address);
            output[4] = static_cast<unsigned char>(this->address >> 8);
            output[5] = static_cast<unsigned char>(this->address >> 16);
            output[6] = static_cast<unsigned char>(this->address >> 24);

            auto bytesToWrite = static_cast<std::uint32_t>(this->buffer.size());
            output[7] = static_cast<unsigned char>(bytesToWrite);
            output[8] = static_cast<unsigned char>(bytesToWrite >> 8);
            output[9] = static_cast<unsigned char>(bytesToWrite >> 16);
            output[10] = static_cast<unsigned char>(bytesToWrite >> 24);

            // We always set the async flag to 0x00 ("write first, then reply")
            output[11] = 0x00;

            output.insert(output.end(), this->buffer.begin(), this->buffer.end());

            return output;
        }
    };

}
