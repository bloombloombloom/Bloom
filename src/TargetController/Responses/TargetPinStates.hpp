#pragma once

#include "Response.hpp"

#include "src/Targets/TargetPinDescriptor.hpp"

namespace Bloom::TargetController::Responses
{
    class TargetPinStates: public Response
    {
    public:
        static constexpr ResponseType type = ResponseType::TARGET_PIN_STATES;

        Targets::TargetPinStateMapping pinStatesByNumber;

        explicit TargetPinStates(const Targets::TargetPinStateMapping& pinStatesByNumber)
            : pinStatesByNumber(pinStatesByNumber)
        {}

        [[nodiscard]] ResponseType getType() const override {
            return TargetPinStates::type;
        }
    };
}
