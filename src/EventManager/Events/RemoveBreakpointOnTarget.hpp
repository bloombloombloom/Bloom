#pragma once

#include <cstdint>

#include "Event.hpp"
#include "src/Targets/TargetBreakpoint.hpp"

namespace Bloom::Events
{
    using Targets::TargetBreakpoint;

    class RemoveBreakpointOnTarget: public Event
    {
    public:
        static inline const std::string name = "RemoveBreakpointOnTarget";
        std::uint32_t address;
        TargetBreakpoint breakpoint;

        std::string getName() const override {
            return RemoveBreakpointOnTarget::name;
        }
    };
}
