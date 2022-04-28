#pragma once

#include "Response.hpp"

#include "src/Targets/TargetState.hpp"

namespace Bloom::TargetController::Responses
{
    class TargetState: public Response
    {
    public:
        static constexpr ResponseType type = ResponseType::TARGET_STATE;

        Targets::TargetState targetState;

        TargetState(Targets::TargetState targetState)
            : targetState(targetState)
        {}

        [[nodiscard]] ResponseType getType() const override {
            return TargetState::type;
        }
    };
}
