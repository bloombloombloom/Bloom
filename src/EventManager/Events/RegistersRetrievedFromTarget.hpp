#pragma once

#include <vector>

#include "Event.hpp"
#include "src/Targets/TargetRegister.hpp"

namespace Bloom::Events
{
    class RegistersRetrievedFromTarget: public Event
    {
    public:
        static inline const std::string name = "RegistersRetrievedFromTarget";
        Targets::TargetRegisters registers;

        std::string getName() const override {
            return RegistersRetrievedFromTarget::name;
        }
    };
}
