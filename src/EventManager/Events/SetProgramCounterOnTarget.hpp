#pragma once

#include <cstdint>
#include <string>

#include "Event.hpp"

namespace Bloom::Events
{
    class SetProgramCounterOnTarget: public Event
    {
    public:
        static inline const std::string name = "SetProgramCounterOnTarget";
        std::uint32_t address;

        std::string getName() const override {
            return SetProgramCounterOnTarget::name;
        }
    };
}
