#pragma once

#include <vector>

#include "Response.hpp"

#include "src/Targets/TargetPinDescriptor.hpp"
#include "src/Targets/TargetGpioPinState.hpp"

#include "src/Helpers/Pair.hpp"

namespace TargetController::Responses
{
    class TargetGpioPinStates: public Response
    {
    public:
        static constexpr ResponseType type = ResponseType::TARGET_GPIO_PIN_STATES;

        Targets::TargetGpioPinDescriptorAndStatePairs gpioPinStates;

        explicit TargetGpioPinStates(const Targets::TargetGpioPinDescriptorAndStatePairs& gpioPinStates)
            : gpioPinStates(gpioPinStates)
        {}

        [[nodiscard]] ResponseType getType() const override {
            return TargetGpioPinStates::type;
        }
    };
}
