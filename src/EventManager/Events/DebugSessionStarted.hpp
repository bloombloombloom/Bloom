#pragma once

#include <string>

#include "Event.hpp"

namespace Bloom::Events
{
    class DebugSessionStarted: public Event
    {
    public:
        static inline const std::string name = "DebugSessionStarted";

        [[nodiscard]] std::string getName() const override {
            return DebugSessionStarted::name;
        }
    };
}
