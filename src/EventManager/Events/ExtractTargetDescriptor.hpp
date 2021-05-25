#pragma once

#include <string>

#include "Event.hpp"

namespace Bloom::Events
{
    class ExtractTargetDescriptor: public Event
    {
    public:
        static inline const std::string name = "ExtractTargetDescriptor";

        std::string getName() const override {
            return ExtractTargetDescriptor::name;
        }
    };
}
