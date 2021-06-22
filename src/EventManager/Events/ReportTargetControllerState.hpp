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

        [[nodiscard]] std::string getName() const override {
            return ReportTargetControllerState::name;
        }
    };
}
