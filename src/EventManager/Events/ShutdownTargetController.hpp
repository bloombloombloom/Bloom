#pragma once

#include <string>

#include "Event.hpp"

namespace Events
{
    class ShutdownTargetController: public Event
    {
    public:
        static constexpr EventType type = EventType::SHUTDOWN_TARGET_CONTROLLER;
        static const inline std::string name = "ShutdownTargetControllerEvent";

        [[nodiscard]] EventType getType() const override {
            return ShutdownTargetController::type;
        }

        [[nodiscard]] std::string getName() const override {
            return ShutdownTargetController::name;
        }
    };
}
