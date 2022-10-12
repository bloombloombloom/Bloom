#pragma once

#include <string>

#include "Event.hpp"

namespace Bloom::Events
{
    class ProgrammingModeEnabled: public Event
    {
    public:
        static constexpr EventType type = EventType::PROGRAMMING_MODE_ENABLED;
        static const inline std::string name = "ProgrammingModeEnabled";

        [[nodiscard]] EventType getType() const override {
            return ProgrammingModeEnabled::type;
        }

        [[nodiscard]] std::string getName() const override {
            return ProgrammingModeEnabled::name;
        }
    };
}
