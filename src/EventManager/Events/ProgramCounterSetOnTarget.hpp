#pragma once

#include <string>

#include "Event.hpp"

namespace Bloom::Events
{
    class ProgramCounterSetOnTarget: public Event
    {
    public:
        static constexpr EventType type = EventType::PROGRAM_COUNTER_SET_ON_TARGET;
        static inline const std::string name = "ProgramCounterSetOnTarget";

        [[nodiscard]] EventType getType() const override {
            return ProgramCounterSetOnTarget::type;
        }

        [[nodiscard]] std::string getName() const override {
            return ProgramCounterSetOnTarget::name;
        }
    };
}
