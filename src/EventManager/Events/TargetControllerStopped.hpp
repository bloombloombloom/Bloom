#pragma once

#include "Event.hpp"
#include "src/Exceptions/Exception.hpp"

namespace Bloom::Events
{
    class TargetControllerStopped: public Event
    {
    private:
        bool stoppedAbruptly = false;
        std::optional<Exceptions::Exception> caughtException;

    public:
        static inline const std::string name = "TargetControllerStoppedEvent";

        std::string getName() const override {
            return TargetControllerStopped::name;
        }
    };
}
