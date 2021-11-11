#pragma once

#include <string>

#include "Event.hpp"

namespace Bloom::Events
{
    class StackPointerRetrievedFromTarget: public Event
    {
    public:
        static inline EventType type = EventType::STACK_POINTER_RETRIEVED_FROM_TARGET;
        static inline const std::string name = "StackPointerRetrievedFromTarget";
        std::uint32_t stackPointer = 0;

        [[nodiscard]] EventType getType() const override {
            return StackPointerRetrievedFromTarget::type;
        }

        [[nodiscard]] std::string getName() const override {
            return StackPointerRetrievedFromTarget::name;
        }
    };
}
