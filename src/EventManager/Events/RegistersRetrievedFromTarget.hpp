#pragma once

#include <vector>

#include "Event.hpp"
#include "src/Targets/TargetRegister.hpp"

namespace Bloom::Events
{
    using Targets::TargetRegisters;

    class RegistersRetrievedFromTarget: public Event
    {
    public:
        static inline const std::string name = "RegistersRetrievedFromTarget";
        TargetRegisters registers;

        std::string getName() const override {
            return RegistersRetrievedFromTarget::name;
        }
    };
}
