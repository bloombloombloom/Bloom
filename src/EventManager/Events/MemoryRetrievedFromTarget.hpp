#pragma once

#include <string>

#include "Event.hpp"
#include "src/Targets/TargetMemory.hpp"

namespace Bloom::Events
{
    class MemoryRetrievedFromTarget: public Event
    {
    public:
        static inline const std::string name = "MemoryRetrievedFromTarget";
        Targets::TargetMemoryBuffer data;

        std::string getName() const override {
            return MemoryRetrievedFromTarget::name;
        }
    };
}
