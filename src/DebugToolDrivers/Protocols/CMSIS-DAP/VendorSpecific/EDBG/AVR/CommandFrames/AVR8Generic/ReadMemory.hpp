#pragma once

#include <cstdint>
#include <set>

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
        std::set<std::uint32_t> excludedAddresses;

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

        void setExcludedAddresses(const std::set<std::uint32_t>& excludedAddresses) {
            this->excludedAddresses = excludedAddresses;
        }

        [[nodiscard]] std::vector<unsigned char> getPayload() const override;
    };
}
