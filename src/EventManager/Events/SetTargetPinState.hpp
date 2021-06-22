#pragma once

#include <string>

#include "Event.hpp"
#include "src/Targets/TargetPinDescriptor.hpp"

namespace Bloom::Events
{
    class SetTargetPinState: public Event
    {
    public:
        static inline const std::string name = "SetTargetPinState";
        int variantId = 0;
        Targets::TargetPinDescriptor pinDescriptor;
        Targets::TargetPinState pinState;

        [[nodiscard]] std::string getName() const override {
            return SetTargetPinState::name;
        }
    };
}
