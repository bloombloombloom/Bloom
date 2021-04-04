#pragma once

#include <cstdint>

#include "Event.hpp"
#include "src/Targets/TargetMemory.hpp"

namespace Bloom::Events
{
    using Bloom::Targets::TargetMemoryBuffer;

    class WriteMemoryToTarget: public Event
    {
    public:
        static inline const std::string name = "WriteMemoryToTarget";
        TargetMemoryType memoryType;
        std::uint32_t startAddress;
        TargetMemoryBuffer buffer;

        std::string getName() const override {
            return WriteMemoryToTarget::name;
        }

        WriteMemoryToTarget() = default;
        WriteMemoryToTarget(TargetMemoryType memoryType, std::uint32_t startAddress, const TargetMemoryBuffer& buffer)
        : memoryType(memoryType), startAddress(startAddress), buffer(buffer) {};
    };
}
