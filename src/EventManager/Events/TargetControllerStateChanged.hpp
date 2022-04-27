#pragma once

#include <string>

#include "Event.hpp"
#include "src/TargetController/TargetControllerState.hpp"

namespace Bloom::Events
{
    class TargetControllerStateChanged: public Event
    {
    public:
        static constexpr EventType type = EventType::TARGET_CONTROLLER_STATE_CHANGED;
        static inline const std::string name = "TargetControllerStateChanged";

        TargetController::TargetControllerState state;
        explicit TargetControllerStateChanged(TargetController::TargetControllerState state)
            : state(state)
        {};

        [[nodiscard]] EventType getType() const override {
            return TargetControllerStateChanged::type;
        }

        [[nodiscard]] std::string getName() const override {
            return TargetControllerStateChanged::name;
        }
    };
}
