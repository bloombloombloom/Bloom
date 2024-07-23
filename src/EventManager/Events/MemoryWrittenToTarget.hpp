#pragma once

#include <string>

#include "Event.hpp"

#include "src/Targets/TargetAddressSpaceDescriptor.hpp"
#include "src/Targets/TargetMemorySegmentDescriptor.hpp"
#include "src/Targets/TargetMemory.hpp"

namespace Events
{
    class MemoryWrittenToTarget: public Event
    {
    public:
        static constexpr EventType type = EventType::MEMORY_WRITTEN_TO_TARGET;
        static const inline std::string name = "MemoryWrittenToTarget";

        const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor;
        const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor;
        Targets::TargetMemoryAddress startAddress;
        Targets::TargetMemorySize size;

        MemoryWrittenToTarget(
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            Targets::TargetMemoryAddress startAddress,
            Targets::TargetMemorySize size
        )
            : addressSpaceDescriptor(addressSpaceDescriptor)
            , memorySegmentDescriptor(memorySegmentDescriptor)
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
