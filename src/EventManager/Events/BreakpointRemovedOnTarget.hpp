#pragma once

#include <string>

#include "Event.hpp"

namespace Bloom::Events
{
    class BreakpointRemovedOnTarget: public Event
    {
    public:
        static inline const std::string name = "BreakpointRemovedOnTarget";

        std::string getName() const override {
            return BreakpointRemovedOnTarget::name;
        }
    };
}
