#pragma once

#include <string>

#include "Event.hpp"

namespace Bloom::Events
{
    class TargetIoPortsUpdated: public Event
    {
    public:
        static inline const std::string name = "TargetIoPortsUpdated";

        std::string getName() const override {
            return TargetIoPortsUpdated::name;
        }
    };
}
