#pragma once

#include "Event.hpp"
#include "src/Helpers/Thread.hpp"

namespace Bloom::Events
{
    class DebugServerThreadStateChanged: public Event
    {
    private:
        ThreadState state;
    public:
        DebugServerThreadStateChanged(ThreadState state): state(state) {};

        static inline const std::string name = "DebugServerThreadStateChanged";

        std::string getName() const override {
            return DebugServerThreadStateChanged::name;
        }

        ThreadState getState() const {
            return this->state;
        }
    };
}
