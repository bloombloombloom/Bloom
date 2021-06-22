#pragma once

#include <string>

#include "Event.hpp"

namespace Bloom::Events
{
    class ShutdownApplication: public Event
    {
    public:
        static inline const std::string name = "ShutdownApplicationEvent";

        [[nodiscard]] std::string getName() const override {
            return ShutdownApplication::name;
        }
    };
}
