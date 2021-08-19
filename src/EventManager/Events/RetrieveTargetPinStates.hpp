#pragma once

#include <string>

#include "Event.hpp"

namespace Bloom::Events
{
    class RetrieveTargetPinStates: public Event
    {
    public:
        static inline EventType type = EventType::RETRIEVE_TARGET_PIN_STATES;
        static inline const std::string name = "RetrieveTargetPinStates";
        int variantId = 0;

        [[nodiscard]] EventType getType() const override {
            return RetrieveTargetPinStates::type;
        }

        [[nodiscard]] std::string getName() const override {
            return RetrieveTargetPinStates::name;
        }
    };
}
