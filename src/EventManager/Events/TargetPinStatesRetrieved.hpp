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
        static inline const std::string name = "TargetPinStatesRetrieved";
        int variantId = 0;
        std::map<int, Targets::TargetPinState> pinSatesByNumber;

        [[nodiscard]] std::string getName() const override {
            return TargetPinStatesRetrieved::name;
        }
    };
}
