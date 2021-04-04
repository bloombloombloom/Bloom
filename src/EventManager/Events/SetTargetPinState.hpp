#pragma once

#include <cstdint>

#include "Event.hpp"
#include "src/Targets/TargetPinDescriptor.hpp"

namespace Bloom::Events
{
    class SetTargetPinState: public Event
    {
    public:
        static inline const std::string name = "SetTargetPinState";
        int variantId;
        Targets::TargetPinDescriptor pinDescriptor;
        Targets::TargetPinState pinState;

        std::string getName() const override {
            return SetTargetPinState::name;
        }
    };
}
