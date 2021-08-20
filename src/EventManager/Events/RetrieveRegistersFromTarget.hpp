#pragma once

#include <string>

#include "Event.hpp"
#include "RegistersRetrievedFromTarget.hpp"
#include "src/Targets/TargetRegister.hpp"

namespace Bloom::Events
{
    class RetrieveRegistersFromTarget: public Event
    {
    public:
        using TargetControllerResponseType = RegistersRetrievedFromTarget;

        static inline EventType type = EventType::RETRIEVE_REGISTERS_FROM_TARGET;
        static inline const std::string name = "RetrieveRegistersFromTarget";
        Targets::TargetRegisterDescriptors descriptors;

        [[nodiscard]] EventType getType() const override {
            return RetrieveRegistersFromTarget::type;
        }

        [[nodiscard]] std::string getName() const override {
            return RetrieveRegistersFromTarget::name;
        }
    };
}
