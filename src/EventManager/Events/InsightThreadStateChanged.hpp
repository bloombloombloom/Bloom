#pragma once

#include <string>

#include "Event.hpp"
#include "src/Helpers/Thread.hpp"

namespace Bloom::Events
{
    class InsightThreadStateChanged: public Event
    {
    public:
        explicit InsightThreadStateChanged(ThreadState state): state(state) {};

        static inline EventType type = EventType::INSIGHT_THREAD_STATE_CHANGED;
        static inline const std::string name = "InsightThreadStateChanged";

        [[nodiscard]] EventType getType() const override {
            return InsightThreadStateChanged::type;
        }

        [[nodiscard]] std::string getName() const override {
            return InsightThreadStateChanged::name;
        }

        ThreadState getState() const {
            return this->state;
        }

    private:
        ThreadState state;
    };
}
