#pragma once

#include <string>

#include "Event.hpp"

namespace Bloom::Events
{
    class TargetControllerErrorOccurred: public Event
    {
    public:
        static inline EventType type = EventType::TARGET_CONTROLLER_ERROR_OCCURRED;
        static inline const std::string name = "TargetControllerErrorOccurred";

        TargetControllerErrorOccurred() = default;

        [[nodiscard]] EventType getType() const override {
            return TargetControllerErrorOccurred::type;
        }

        [[nodiscard]] std::string getName() const override {
            return TargetControllerErrorOccurred::name;
        }
    };
}
