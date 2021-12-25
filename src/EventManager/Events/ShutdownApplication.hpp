#pragma once

#include <string>

#include "Event.hpp"

namespace Bloom::Events
{
    class ShutdownApplication: public Event
    {
    public:
        static constexpr EventType type = EventType::SHUTDOWN_APPLICATION;
        static inline const std::string name = "ShutdownApplicationEvent";

        [[nodiscard]] EventType getType() const override {
            return ShutdownApplication::type;
        }

        [[nodiscard]] std::string getName() const override {
            return ShutdownApplication::name;
        }
    };
}
