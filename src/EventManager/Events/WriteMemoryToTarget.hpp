#pragma once

#include <cstdint>
#include <string>
#include <utility>

#include "Event.hpp"
#include "src/Targets/TargetMemory.hpp"

namespace Bloom::Events
{
    class WriteMemoryToTarget: public Event
    {
    public:
        static inline const std::string name = "WriteMemoryToTarget";
        Targets::TargetMemoryType memoryType = Targets::TargetMemoryType::RAM;
        std::uint32_t startAddress = 0;
        Targets::TargetMemoryBuffer buffer;

        [[nodiscard]] std::string getName() const override {
            return WriteMemoryToTarget::name;
        }

        WriteMemoryToTarget() = default;
        WriteMemoryToTarget(
            Targets::TargetMemoryType memoryType,
            std::uint32_t startAddress,
            Targets::TargetMemoryBuffer buffer
        ): memoryType(memoryType), startAddress(startAddress), buffer(std::move(buffer)) {};
    };
}
