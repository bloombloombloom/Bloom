#pragma once

#include <string>

#include "Event.hpp"

namespace Bloom::Events
{
    class TargetIoPortsUpdated: public Event
    {
    public:
        static inline EventType type = EventType::TARGET_IO_PORTS_UPDATED;
        static inline const std::string name = "TargetIoPortsUpdated";

        [[nodiscard]] EventType getType() const override {
            return TargetIoPortsUpdated::type;
        }

        [[nodiscard]] std::string getName() const override {
            return TargetIoPortsUpdated::name;
        }
    };
}
