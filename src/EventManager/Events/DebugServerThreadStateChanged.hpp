#pragma once

#include <string>

#include "Event.hpp"
#include "src/Helpers/Thread.hpp"

namespace Events
{
    class DebugServerThreadStateChanged: public Event
    {
    public:
        static constexpr EventType type = EventType::DEBUG_SERVER_THREAD_STATE_CHANGED;
        static const inline std::string name = "DebugServerThreadStateChanged";

        explicit DebugServerThreadStateChanged(ThreadState state)
            : state(state)
        {};

        [[nodiscard]] EventType getType() const override {
            return DebugServerThreadStateChanged::type;
        }

        [[nodiscard]] std::string getName() const override {
            return DebugServerThreadStateChanged::name;
        }

        [[nodiscard]] ThreadState getState() const {
            return this->state;
        }

    private:
        ThreadState state;
    };
}
