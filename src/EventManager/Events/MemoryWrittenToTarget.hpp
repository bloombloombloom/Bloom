#pragma once

#include <string>

#include "Event.hpp"
#include "src/Targets/TargetMemory.hpp"

namespace Bloom::Events
{
    class MemoryWrittenToTarget: public Event
    {
    public:
        static constexpr EventType type = EventType::MEMORY_WRITTEN_TO_TARGET;
        static inline const std::string name = "MemoryWrittenToTarget";

        Targets::TargetMemoryType memoryType;
        std::uint32_t startAddress;
        std::uint32_t size;

        MemoryWrittenToTarget(
            Targets::TargetMemoryType memoryType,
            std::uint32_t startAddress,
            std::uint32_t size
        )
            : memoryType(memoryType)
            , startAddress(startAddress)
            , size(size)
        {};

        [[nodiscard]] EventType getType() const override {
            return MemoryWrittenToTarget::type;
        }

        [[nodiscard]] std::string getName() const override {
            return MemoryWrittenToTarget::name;
        }
    };
}
