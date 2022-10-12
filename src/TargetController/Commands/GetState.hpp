#pragma once

#include "Command.hpp"

#include "src/TargetController/Responses/State.hpp"

namespace Bloom::TargetController::Commands
{
    class GetState: public Command
    {
    public:
        using SuccessResponseType = Responses::State;

        static constexpr CommandType type = CommandType::GET_STATE;
        static const inline std::string name = "GetState";

        [[nodiscard]] CommandType getType() const override {
            return GetState::type;
        }

        [[nodiscard]] bool requiresActiveState() const override {
            return false;
        }

        [[nodiscard]] bool requiresDebugMode() const override {
            return false;
        }
    };
}
