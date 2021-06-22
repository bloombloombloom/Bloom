#pragma once

#include <cstdint>

#include "Avr8GenericCommandFrame.hpp"
#include "../../ResponseFrames/AVR8Generic/ReadMemory.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::Avr8Generic
{
    class ReadMemory: public Avr8GenericCommandFrame
    {
    private:
        Avr8MemoryType type = Avr8MemoryType::SRAM;
        std::uint32_t address = 0;
        std::uint32_t bytes = 0;

    public:
        using ResponseFrameType = ResponseFrames::Avr8Generic::ReadMemory;

        ReadMemory() = default;

        void setType(const Avr8MemoryType& type) {
            this->type = type;
        }

        void setAddress(std::uint32_t address) {
            this->address = address;
        }

        void setBytes(std::uint32_t bytes) {
            this->bytes = bytes;
        }

        [[nodiscard]] std::vector<unsigned char> getPayload() const override {
            /*
             * The read memory command consists of 11 bytes:
             * 1. Command ID (0x21)
             * 2. Version (0x00)
             * 3. Memory type (Avr8MemoryType)
             * 4. Start address (4 bytes)
             * 5. Number of bytes to read (4 bytes)
             */
            auto output = std::vector<unsigned char>(11, 0x00);
            output[0] = 0x21;
            output[1] = 0x00;
            output[2] = static_cast<unsigned char>(this->type);
            output[3] = static_cast<unsigned char>(this->address);
            output[4] = static_cast<unsigned char>(this->address >> 8);
            output[5] = static_cast<unsigned char>(this->address >> 16);
            output[6] = static_cast<unsigned char>(this->address >> 24);
            output[7] = static_cast<unsigned char>(this->bytes);
            output[8] = static_cast<unsigned char>(this->bytes >> 8);
            output[9] = static_cast<unsigned char>(this->bytes >> 16);
            output[10] = static_cast<unsigned char>(this->bytes >> 24);

            return output;
        }
    };
}
