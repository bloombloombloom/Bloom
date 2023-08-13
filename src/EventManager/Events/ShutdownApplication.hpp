#pragma once

#include <string>

#include "Event.hpp"

namespace Events
{
    class ShutdownApplication: public Event
    {
    public:
        static constexpr EventType type = EventType::SHUTDOWN_APPLICATION;
        static const inline std::string name = "ShutdownApplicationEvent";

        [[nodiscard]] EventType getType() const override {
            return ShutdownApplication::type;
        }

        [[nodiscard]] std::string getName() const override {
            return ShutdownApplication::name;
        }
    };
}
