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
        static inline EventType type = EventType::WRITE_MEMORY_TO_TARGET;
        static inline const std::string name = "WriteMemoryToTarget";
        Targets::TargetMemoryType memoryType = Targets::TargetMemoryType::RAM;
        std::uint32_t startAddress = 0;
        Targets::TargetMemoryBuffer buffer;

        WriteMemoryToTarget() = default;
        WriteMemoryToTarget(
            Targets::TargetMemoryType memoryType,
            std::uint32_t startAddress,
            Targets::TargetMemoryBuffer buffer
        ): memoryType(memoryType), startAddress(startAddress), buffer(std::move(buffer)) {};

        [[nodiscard]] EventType getType() const override {
            return WriteMemoryToTarget::type;
        }

        [[nodiscard]] std::string getName() const override {
            return WriteMemoryToTarget::name;
        }
    };
}
