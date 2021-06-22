#pragma once

#include <string>

#include "Event.hpp"

namespace Bloom::Events
{
    class ResetTarget: public Event
    {
    public:
        static inline const std::string name = "ResetTargetEvent";

        [[nodiscard]] std::string getName() const override {
            return ResetTarget::name;
        }
    };
}
