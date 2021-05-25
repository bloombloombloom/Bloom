#pragma once

#include <string>

#include "Event.hpp"
#include "src/Helpers/Thread.hpp"

namespace Bloom::Events
{
    class InsightStateChanged: public Event
    {
    private:
        ThreadState state;
    public:
        InsightStateChanged(ThreadState state): state(state) {

        };

        static inline const std::string name = "InsightStateChanged";

        std::string getName() const override {
            return InsightStateChanged::name;
        }

        ThreadState getState() const {
            return this->state;
        }
    };
}
