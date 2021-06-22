#pragma once

#include <cstdint>
#include <string>

#include "Event.hpp"
#include "src/Targets/TargetMemory.hpp"

namespace Bloom::Events
{
    class RetrieveMemoryFromTarget: public Event
    {
    public:
        static inline const std::string name = "RetrieveMemoryFromTarget";
        Targets::TargetMemoryType memoryType = Targets::TargetMemoryType::RAM;
        std::uint32_t startAddress = 0;
        std::uint32_t bytes = 0;

        [[nodiscard]] std::string getName() const override {
            return RetrieveMemoryFromTarget::name;
        }
    };
}
