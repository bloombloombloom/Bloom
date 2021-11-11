#pragma once

#include <string>

#include "Event.hpp"
#include "StackPointerRetrievedFromTarget.hpp"

namespace Bloom::Events
{
    class RetrieveStackPointerFromTarget: public Event
    {
    public:
        using TargetControllerResponseType = StackPointerRetrievedFromTarget;

        static inline EventType type = EventType::RETRIEVE_STACK_POINTER_FROM_TARGET;
        static inline const std::string name = "RetrieveStackPointerFromTarget";

        [[nodiscard]] EventType getType() const override {
            return RetrieveStackPointerFromTarget::type;
        }

        [[nodiscard]] std::string getName() const override {
            return RetrieveStackPointerFromTarget::name;
        }
    };
}
