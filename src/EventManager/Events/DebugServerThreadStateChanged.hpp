#pragma once

#include <string>

#include "Event.hpp"
#include "src/Helpers/Thread.hpp"

namespace Bloom::Events
{
    class DebugServerThreadStateChanged: public Event
    {
    private:
        ThreadState state;
    public:
        explicit DebugServerThreadStateChanged(ThreadState state): state(state) {};

        static inline const std::string name = "DebugServerThreadStateChanged";

        [[nodiscard]] std::string getName() const override {
            return DebugServerThreadStateChanged::name;
        }

        [[nodiscard]] ThreadState getState() const {
            return this->state;
        }
    };
}
