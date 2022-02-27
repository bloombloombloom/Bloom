#pragma once

#include <cstdint>
#include <set>

#include "Avr8GenericCommandFrame.hpp"
#include "../../ResponseFrames/AVR8Generic/ReadMemory.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::Avr8Generic
{
    class ReadMemory: public Avr8GenericCommandFrame<std::vector<unsigned char>>
    {
    public:
        using ResponseFrameType = ResponseFrames::Avr8Generic::ReadMemory;

        ReadMemory(
            const Avr8MemoryType& type,
            std::uint32_t address,
            std::uint32_t bytes,
            const std::set<std::uint32_t>& excludedAddresses = {}
        );
    };
}
