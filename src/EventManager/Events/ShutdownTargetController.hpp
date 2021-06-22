#pragma once

#include <string>

#include "Event.hpp"

namespace Bloom::Events
{
    class ShutdownTargetController: public Event
    {
    public:
        static inline const std::string name = "ShutdownTargetControllerEvent";

        [[nodiscard]] std::string getName() const override {
            return ShutdownTargetController::name;
        }
    };
}
