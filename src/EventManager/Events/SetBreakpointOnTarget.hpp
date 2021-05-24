#pragma once

#include <cstdint>

#include "Event.hpp"
#include "src/Targets/TargetBreakpoint.hpp"

namespace Bloom::Events
{
    class SetBreakpointOnTarget: public Event
    {
    public:
        static inline const std::string name = "SetBreakpointOnTarget";
        std::uint32_t address;
        Targets::TargetBreakpoint breakpoint;

        std::string getName() const override {
            return SetBreakpointOnTarget::name;
        }
    };
}
