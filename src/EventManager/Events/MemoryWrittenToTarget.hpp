#pragma once

#include <string>

#include "Event.hpp"
#include "src/Targets/TargetMemory.hpp"

namespace Events
{
    class MemoryWrittenToTarget: public Event
    {
    public:
        static constexpr EventType type = EventType::MEMORY_WRITTEN_TO_TARGET;
        static const inline std::string name = "MemoryWrittenToTarget";

        Targets::TargetMemoryType memoryType;
        Targets::TargetMemoryAddress startAddress;
        Targets::TargetMemorySize size;

        MemoryWrittenToTarget(
            Targets::TargetMemoryType memoryType,
            Targets::TargetMemoryAddress startAddress,
            Targets::TargetMemorySize size
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
