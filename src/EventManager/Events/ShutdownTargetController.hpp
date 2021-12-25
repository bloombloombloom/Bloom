#pragma once

#include <string>

#include "Event.hpp"

namespace Bloom::Events
{
    class ShutdownTargetController: public Event
    {
    public:
        static constexpr EventType type = EventType::SHUTDOWN_TARGET_CONTROLLER;
        static inline const std::string name = "ShutdownTargetControllerEvent";

        [[nodiscard]] EventType getType() const override {
            return ShutdownTargetController::type;
        }

        [[nodiscard]] std::string getName() const override {
            return ShutdownTargetController::name;
        }
    };
}
