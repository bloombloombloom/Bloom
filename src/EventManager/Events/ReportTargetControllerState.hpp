#pragma once

#include <string>

#include "Event.hpp"

namespace Bloom::Events
{
    class ReportTargetControllerState: public Event
    {
    public:
        static inline EventType type = EventType::REPORT_TARGET_CONTROLLER_STATE;
        static inline const std::string name = "ReportTargetControllerState";

        ReportTargetControllerState() {};

        [[nodiscard]] EventType getType() const override {
            return ReportTargetControllerState::type;
        }

        [[nodiscard]] std::string getName() const override {
            return ReportTargetControllerState::name;
        }
    };
}
