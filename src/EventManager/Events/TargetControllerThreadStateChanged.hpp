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
        explicit TargetControllerThreadStateChanged(ThreadState state): state(state) {};

        static inline const std::string name = "TargetControllerThreadStateChanged";

        [[nodiscard]] std::string getName() const override {
            return TargetControllerThreadStateChanged::name;
        }

        ThreadState getState() const {
            return this->state;
        }
    };
}
