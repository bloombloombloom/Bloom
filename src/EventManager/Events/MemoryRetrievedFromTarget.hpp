#pragma once

#include <vector>

#include "Event.hpp"
#include "src/Targets/TargetMemory.hpp"

namespace Bloom::Events
{
    using Targets::TargetMemoryBuffer;
    class MemoryRetrievedFromTarget: public Event
    {
    public:
        static inline const std::string name = "MemoryRetrievedFromTarget";
        TargetMemoryBuffer data;

        std::string getName() const override {
            return MemoryRetrievedFromTarget::name;
        }
    };
}
