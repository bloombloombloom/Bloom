#pragma once

#include <cstdint>
#include <string>

#include "Event.hpp"

namespace Bloom::Events
{
    class InsightActivationRequested: public Event
    {
    public:
        static constexpr EventType type = EventType::INSIGHT_ACTIVATION_REQUESTED;
        static const inline std::string name = "InsightActivationRequested";

        InsightActivationRequested() = default;

        [[nodiscard]] EventType getType() const override {
            return InsightActivationRequested::type;
        }

        [[nodiscard]] std::string getName() const override {
            return InsightActivationRequested::name;
        }
    };
}
