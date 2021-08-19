#pragma once

#include <string>

#include "Event.hpp"
#include "src/Helpers/Thread.hpp"

namespace Bloom::Events
{
    class TargetControllerThreadStateChanged: public Event
    {
    private:
        ThreadState state;

    public:
        static inline EventType type = EventType::TARGET_CONTROLLER_THREAD_STATE_CHANGED;
        static inline const std::string name = "TargetControllerThreadStateChanged";

        explicit TargetControllerThreadStateChanged(ThreadState state): state(state) {};

        [[nodiscard]] EventType getType() const override {
            return TargetControllerThreadStateChanged::type;
        }

        [[nodiscard]] std::string getName() const override {
            return TargetControllerThreadStateChanged::name;
        }

        ThreadState getState() const {
            return this->state;
        }
    };
}
