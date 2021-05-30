#pragma once

#include <string>

#include "Event.hpp"

namespace Bloom::Events
{
    class ReportTargetControllerState: public Event
    {
    public:
        ReportTargetControllerState() {};

        static inline const std::string name = "ReportTargetControllerState";

        std::string getName() const override {
            return ReportTargetControllerState::name;
        }
    };
}
