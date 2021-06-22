#pragma once

#include <cstdint>
#include <string>

#include "Event.hpp"
#include "src/Targets/TargetBreakpoint.hpp"

namespace Bloom::Events
{
    class SetBreakpointOnTarget: public Event
    {
    public:
        static inline const std::string name = "SetBreakpointOnTarget";
        Targets::TargetBreakpoint breakpoint;

        [[nodiscard]] std::string getName() const override {
            return SetBreakpointOnTarget::name;
        }
    };
}
