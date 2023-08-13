#pragma once

#include <cstdint>

#include "MemorySegment.hpp"

namespace Targets::TargetDescription
{
    struct AddressSpace
    {
        std::string id;
        std::string name;
        std::uint32_t startAddress;
        std::uint32_t size;
        bool littleEndian = true;
        std::map<MemorySegmentType, std::map<std::string, MemorySegment>> memorySegmentsByTypeAndName;
    };
}
