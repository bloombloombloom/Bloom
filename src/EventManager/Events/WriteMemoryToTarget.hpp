#pragma once

#include <cstdint>
#include <string>

#include "Event.hpp"
#include "src/Targets/TargetMemory.hpp"

namespace Bloom::Events
{
    class WriteMemoryToTarget: public Event
    {
    public:
        static inline const std::string name = "WriteMemoryToTarget";
        Targets::TargetMemoryType memoryType;
        std::uint32_t startAddress;
        Targets::TargetMemoryBuffer buffer;

        std::string getName() const override {
            return WriteMemoryToTarget::name;
        }

        WriteMemoryToTarget() = default;
        WriteMemoryToTarget(
            Targets::TargetMemoryType memoryType,
            std::uint32_t startAddress,
            const Targets::TargetMemoryBuffer& buffer
        ): memoryType(memoryType), startAddress(startAddress), buffer(buffer) {};
    };
}
