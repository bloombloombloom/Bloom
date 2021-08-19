#pragma once

#include <string>

#include "Event.hpp"

namespace Bloom::Events
{
    class RegistersWrittenToTarget: public Event
    {
    public:
        static inline EventType type = EventType::REGISTERS_WRITTEN_TO_TARGET;
        static inline const std::string name = "RegistersWrittenToTarget";

        [[nodiscard]] EventType getType() const override {
            return RegistersWrittenToTarget::type;
        }

        [[nodiscard]] std::string getName() const override {
            return RegistersWrittenToTarget::name;
        }
    };
}
