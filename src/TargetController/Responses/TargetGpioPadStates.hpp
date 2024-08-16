#pragma once

#include <vector>

#include "Response.hpp"

#include "src/Targets/TargetPinDescriptor.hpp"
#include "src/Targets/TargetGpioPadState.hpp"

#include "src/Helpers/Pair.hpp"

namespace TargetController::Responses
{
    class TargetGpioPadStates: public Response
    {
    public:
        static constexpr ResponseType type = ResponseType::TARGET_GPIO_PAD_STATES;

        Targets::TargetGpioPadDescriptorAndStatePairs gpioPadStates;

        explicit TargetGpioPadStates(const Targets::TargetGpioPadDescriptorAndStatePairs& gpioPadStates)
            : gpioPadStates(gpioPadStates)
        {}

        [[nodiscard]] ResponseType getType() const override {
            return TargetGpioPadStates::type;
        }
    };
}
