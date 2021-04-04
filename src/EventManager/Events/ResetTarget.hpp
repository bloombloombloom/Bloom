#pragma once

#include "Event.hpp"

namespace Bloom::Events
{
    class ResetTarget: public Event
    {
    public:
        static inline const std::string name = "ResetTargetEvent";

        std::string getName() const override {
            return ResetTarget::name;
        }
    };
}
