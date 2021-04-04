#pragma once

#include "Event.hpp"

namespace Bloom::Events
{
    class DebugSessionFinished: public Event
    {
    public:
        static inline const std::string name = "DebugSessionFinished";

        std::string getName() const override {
            return DebugSessionFinished::name;
        }
    };
}
