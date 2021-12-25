#pragma once

#include <string>

#include "Event.hpp"

namespace Bloom::Events
{
    class ResetTarget: public Event
    {
    public:
        static constexpr EventType type = EventType::RESET_TARGET;
        static inline const std::string name = "ResetTargetEvent";

        [[nodiscard]] EventType getType() const override {
            return ResetTarget::type;
        }

        [[nodiscard]] std::string getName() const override {
            return ResetTarget::name;
        }
    };
}
