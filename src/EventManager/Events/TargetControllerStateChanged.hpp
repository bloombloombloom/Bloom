#pragma once

#include "Event.hpp"
#include "src/Helpers/Thread.hpp"

namespace Bloom::Events
{

    class TargetControllerStateChanged: public Event
    {
    private:
        ThreadState state;
    public:
        TargetControllerStateChanged(ThreadState state): state(state) {

        };

        static inline const std::string name = "TargetControllerStateChanged";

        std::string getName() const override {
            return TargetControllerStateChanged::name;
        }

        ThreadState getState() const {
            return this->state;
        }
    };
}
