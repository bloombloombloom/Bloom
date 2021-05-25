#pragma once

#include <cstdint>
#include <string>

#include "Event.hpp"
#include "src/Targets/TargetBreakpoint.hpp"

namespace Bloom::Events
{
    class RemoveBreakpointOnTarget: public Event
    {
    public:
        static inline const std::string name = "RemoveBreakpointOnTarget";
        std::uint32_t address;
        Targets::TargetBreakpoint breakpoint;

        std::string getName() const override {
            return RemoveBreakpointOnTarget::name;
        }
    };
}
