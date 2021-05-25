#pragma once

#include <string>

#include "Event.hpp"

namespace Bloom::Events
{
    class RetrieveTargetPinStates: public Event
    {
    public:
        static inline const std::string name = "RetrieveTargetPinStates";
        int variantId;

        std::string getName() const override {
            return RetrieveTargetPinStates::name;
        }
    };
}
