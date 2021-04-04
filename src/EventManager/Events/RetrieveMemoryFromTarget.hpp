#pragma once

#include <cstdint>

#include "Event.hpp"
#include "src/Targets/TargetMemory.hpp"

namespace Bloom::Events
{
    using Targets::TargetMemoryType;

    class RetrieveMemoryFromTarget: public Event
    {
    public:
        static inline const std::string name = "RetrieveMemoryFromTarget";
        TargetMemoryType memoryType = TargetMemoryType::RAM;
        std::uint32_t startAddress;
        std::uint32_t bytes;

        std::string getName() const override {
            return RetrieveMemoryFromTarget::name;
        }
    };
}
