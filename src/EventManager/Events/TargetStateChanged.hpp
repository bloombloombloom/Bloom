#pragma once

#include <string>

#include "Event.hpp"
#include "src/Targets/TargetState.hpp"

namespace Events
{
    class TargetStateChanged: public Event
    {
    public:
        static constexpr EventType type = EventType::TARGET_STATE_CHANGED;
        static const inline std::string name = "TargetStateChanged";

        Targets::TargetState newState;
        Targets::TargetState previousState;

        explicit TargetStateChanged(const Targets::TargetState& newState, const Targets::TargetState& previousState)
            : newState(newState)
            , previousState(previousState)
        {};

        [[nodiscard]] EventType getType() const override {
            return TargetStateChanged::type;
        }

        [[nodiscard]] std::string getName() const override {
            return TargetStateChanged::name;
        }
    };
}
