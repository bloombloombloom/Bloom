#pragma once

#include <string>

#include "Event.hpp"
#include "src/Helpers/Thread.hpp"

namespace Events
{
    class TargetControllerThreadStateChanged: public Event
    {
    public:
        static constexpr EventType type = EventType::TARGET_CONTROLLER_THREAD_STATE_CHANGED;
        static const inline std::string name = "TargetControllerThreadStateChanged";

        explicit TargetControllerThreadStateChanged(ThreadState state)
            : state(state)
        {};

        [[nodiscard]] EventType getType() const override {
            return TargetControllerThreadStateChanged::type;
        }

        [[nodiscard]] std::string getName() const override {
            return TargetControllerThreadStateChanged::name;
        }

        ThreadState getState() const {
            return this->state;
        }

    private:
        ThreadState state;
    };
}
