#pragma once

#include <set>

#include "Event.hpp"
#include "src/Targets/TargetRegister.hpp"

namespace Bloom::Events
{
    class RetrieveRegistersFromTarget: public Event
    {
    public:
        static inline const std::string name = "RetrieveRegistersFromTarget";
        Targets::TargetRegisterDescriptors descriptors;

        std::string getName() const override {
            return RetrieveRegistersFromTarget::name;
        }
    };
}
