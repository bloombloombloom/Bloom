#pragma once

#include <cstdint>
#include <set>

#include "Avr8GenericCommandFrame.hpp"
#include "../../ResponseFrames/Avr8Generic/ReadMemory.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr::CommandFrames::Avr8Generic
{
    class ReadMemory: public Avr8GenericCommandFrame<std::vector<unsigned char>>
    {
    public:
        using ExpectedResponseFrameType = ResponseFrames::Avr8Generic::ReadMemory;

        ReadMemory(
            const Avr8MemoryType& type,
            std::uint32_t address,
            std::uint32_t bytes,
            const std::set<std::uint32_t>& excludedAddresses = {}
        );
    };
}
