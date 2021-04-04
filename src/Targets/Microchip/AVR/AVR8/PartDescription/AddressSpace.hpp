#pragma once

#include <cstdint>
#include "MemorySegment.hpp"

namespace Bloom::Targets::Microchip::Avr::Avr8Bit::PartDescription
{
    struct AddressSpace {
        std::string id;
        std::string name;
        std::uint16_t startAddress;
        std::uint16_t size;
        bool littleEndian = true;
        std::map<MemorySegmentType, std::map<std::string, MemorySegment>> memorySegmentsByTypeAndName;
    };
}
