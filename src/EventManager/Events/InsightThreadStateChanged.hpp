#pragma once

#include <string>

#include "Event.hpp"
#include "src/Helpers/Thread.hpp"

namespace Bloom::Events
{
    class InsightThreadStateChanged: public Event
    {
    private:
        ThreadState state;
    public:
        InsightThreadStateChanged(ThreadState state): state(state) {};

        static inline const std::string name = "InsightThreadStateChanged";

        std::string getName() const override {
            return InsightThreadStateChanged::name;
        }

        ThreadState getState() const {
            return this->state;
        }
    };
}
