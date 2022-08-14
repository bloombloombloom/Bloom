#pragma once

#include "Response.hpp"

#include "src/TargetController/TargetControllerState.hpp"

namespace Bloom::TargetController::Responses
{
    class State: public Response
    {
    public:
        static constexpr ResponseType type = ResponseType::STATE;

        TargetControllerState state;

        explicit State(TargetControllerState state)
            : state(state)
        {}

        [[nodiscard]] ResponseType getType() const override {
            return State::type;
        }
    };
}
