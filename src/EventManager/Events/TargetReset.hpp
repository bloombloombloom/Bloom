#pragma once

#include <string>

#include "Event.hpp"

namespace Bloom::Events
{
    class TargetReset: public Event
    {
    public:
        static constexpr EventType type = EventType::TARGET_RESET;
        static const inline std::string name = "TargetReset";

        [[nodiscard]] EventType getType() const override {
            return TargetReset::type;
        }

        [[nodiscard]] std::string getName() const override {
            return TargetReset::name;
        }
    };
}
