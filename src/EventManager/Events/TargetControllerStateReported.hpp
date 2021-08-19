#pragma once

#include <string>

#include "Event.hpp"
#include "src/TargetController/TargetControllerState.hpp"

namespace Bloom::Events
{
    class TargetControllerStateReported: public Event
    {
    public:
        static inline EventType type = EventType::TARGET_CONTROLLER_STATE_REPORTED;
        static inline const std::string name = "TargetControllerStateReported";

        TargetControllerState state;
        explicit TargetControllerStateReported(TargetControllerState state): state(state) {};

        [[nodiscard]] EventType getType() const override {
            return TargetControllerStateReported::type;
        }

        [[nodiscard]] std::string getName() const override {
            return TargetControllerStateReported::name;
        }
    };
}
