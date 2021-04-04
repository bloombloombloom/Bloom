#pragma once

#include "Event.hpp"

namespace Bloom::Events
{
    class TargetExecutionResumed: public Event
    {
    public:
        static inline const std::string name = "TargetExecutionResumed";

        TargetExecutionResumed() = default;

        std::string getName() const override {
            return TargetExecutionResumed::name;
        }
    };
}
