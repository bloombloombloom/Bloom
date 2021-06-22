#pragma once

#include <string>

#include "Event.hpp"

namespace Bloom::Events
{
    class ShutdownDebugServer: public Event
    {
    public:
        static inline const std::string name = "ShutdownDebugServer";

        [[nodiscard]] std::string getName() const override {
            return ShutdownDebugServer::name;
        }
    };
}
