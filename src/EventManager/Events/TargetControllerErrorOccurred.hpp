#pragma once

#include "Event.hpp"

namespace Bloom::Events
{
    class TargetControllerErrorOccurred: public Event
    {
    public:
        static inline const std::string name = "TargetControllerErrorOccurred";

        TargetControllerErrorOccurred() = default;

        std::string getName() const override {
            return TargetControllerErrorOccurred::name;
        }
    };
}
