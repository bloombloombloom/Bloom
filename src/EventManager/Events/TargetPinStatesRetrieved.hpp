#pragma once

#include <map>
#include <string>

#include "Event.hpp"
#include "src/Targets/TargetPinDescriptor.hpp"

namespace Bloom::Events
{
    class TargetPinStatesRetrieved: public Event
    {
    public:
        static inline EventType type = EventType::TARGET_PIN_STATES_RETRIEVED;
        static inline const std::string name = "TargetPinStatesRetrieved";
        int variantId = 0;
        std::map<int, Targets::TargetPinState> pinSatesByNumber;

        [[nodiscard]] EventType getType() const override {
            return TargetPinStatesRetrieved::type;
        }

        [[nodiscard]] std::string getName() const override {
            return TargetPinStatesRetrieved::name;
        }
    };
}
