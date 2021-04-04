#pragma once

#include <src/Helpers/Thread.hpp>
#include "Event.hpp"

namespace Bloom::Events
{
    class DebugServerStateChanged: public Event
    {
    private:
        ThreadState state;
    public:
        DebugServerStateChanged(ThreadState state) : state(state) {};

        static inline const std::string name = "DebugServerStateChanged";

        std::string getName() const override {
            return DebugServerStateChanged::name;
        }

        ThreadState getState() const {
            return this->state;
        }
    };
}
