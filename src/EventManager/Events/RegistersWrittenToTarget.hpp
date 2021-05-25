#pragma once

#include <string>

#include "Event.hpp"

namespace Bloom::Events
{
    class RegistersWrittenToTarget: public Event
    {
    public:
        static inline const std::string name = "RegistersWrittenToTarget";

        std::string getName() const override {
            return RegistersWrittenToTarget::name;
        }
    };
}
