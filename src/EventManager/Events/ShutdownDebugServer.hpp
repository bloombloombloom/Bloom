#pragma once

#include <string>

#include "Event.hpp"

namespace Bloom::Events
{
    class ShutdownDebugServer: public Event
    {
    public:
        static constexpr EventType type = EventType::SHUTDOWN_DEBUG_SERVER;
        static const inline std::string name = "ShutdownDebugServer";

        [[nodiscard]] EventType getType() const override {
            return ShutdownDebugServer::type;
        }

        [[nodiscard]] std::string getName() const override {
            return ShutdownDebugServer::name;
        }
    };
}
