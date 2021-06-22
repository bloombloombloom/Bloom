#pragma once

#include <string>

#include "Event.hpp"

namespace Bloom::Events
{
    class BreakpointSetOnTarget: public Event
    {
    public:
        static inline const std::string name = "BreakpointSetOnTarget";

        [[nodiscard]] std::string getName() const override {
            return BreakpointSetOnTarget::name;
        }
    };
}
