#pragma once

#include "Command.hpp"
#include "src/TargetController/Responses/TargetState.hpp"

namespace Bloom::TargetController::Commands
{
    class GetTargetState: public Command
    {
    public:
        using SuccessResponseType = Responses::TargetState;

        static constexpr CommandType type = CommandType::GET_TARGET_STATE;
        static inline const std::string name = "GetTargetState";

        [[nodiscard]] CommandType getType() const override {
            return GetTargetState::type;
        }
    };
}
