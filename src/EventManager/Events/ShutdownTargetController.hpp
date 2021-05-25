#pragma once

#include <string>

#include "Event.hpp"

namespace Bloom::Events
{
    class ShutdownTargetController: public Event
    {
    public:
        static inline const std::string name = "ShutdownTargetControllerEvent";

        std::string getName() const override {
            return ShutdownTargetController::name;
        }
    };
}
