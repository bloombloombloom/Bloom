#pragma once

#include "Event.hpp"

namespace Bloom::Events
{
    class ProgramCounterSetOnTarget: public Event
    {
    public:
        static inline const std::string name = "ProgramCounterSetOnTarget";

        std::string getName() const override {
            return ProgramCounterSetOnTarget::name;
        }
    };
}
