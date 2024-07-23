#pragma once

#include "Response.hpp"

#include "src/Targets/TargetState.hpp"

namespace TargetController::Responses
{
    class TargetState: public Response
    {
    public:
        static constexpr ResponseType type = ResponseType::TARGET_STATE;

        const Targets::TargetState& targetState;

        explicit TargetState(const Targets::TargetState& targetState)
            : targetState(targetState)
        {}

        [[nodiscard]] ResponseType getType() const override {
            return TargetState::type;
        }
    };
}
