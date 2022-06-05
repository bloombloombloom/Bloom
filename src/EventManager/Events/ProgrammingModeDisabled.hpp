#pragma once

#include <string>

#include "Event.hpp"

namespace Bloom::Events
{
    class ProgrammingModeDisabled: public Event
    {
    public:
        static constexpr EventType type = EventType::PROGRAMMING_MODE_DISABLED;
        static inline const std::string name = "ProgrammingModeDisabled";

        [[nodiscard]] EventType getType() const override {
            return ProgrammingModeDisabled::type;
        }

        [[nodiscard]] std::string getName() const override {
            return ProgrammingModeDisabled::name;
        }
    };
}
