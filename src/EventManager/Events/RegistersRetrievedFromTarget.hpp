#pragma once

#include <string>

#include "Event.hpp"
#include "src/Targets/TargetRegister.hpp"

namespace Bloom::Events
{
    class RegistersRetrievedFromTarget: public Event
    {
    public:
        static inline EventType type = EventType::REGISTERS_RETRIEVED_FROM_TARGET;
        static inline const std::string name = "RegistersRetrievedFromTarget";
        Targets::TargetRegisters registers;

        [[nodiscard]] EventType getType() const override {
            return RegistersRetrievedFromTarget::type;
        }

        [[nodiscard]] std::string getName() const override {
            return RegistersRetrievedFromTarget::name;
        }
    };
}
