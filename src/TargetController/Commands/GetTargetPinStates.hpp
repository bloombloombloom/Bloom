#pragma once

#include "Command.hpp"

#include "src/TargetController/Responses/TargetPinStates.hpp"

namespace Bloom::TargetController::Commands
{
    class GetTargetPinStates: public Command
    {
    public:
        using SuccessResponseType = Responses::TargetPinStates;

        static constexpr CommandType type = CommandType::GET_TARGET_PIN_STATES;
        static const inline std::string name = "GetTargetPinStates";

        int variantId = 0;

        explicit GetTargetPinStates(int variantId)
            : variantId(variantId)
        {};

        [[nodiscard]] CommandType getType() const override {
            return GetTargetPinStates::type;
        }

        [[nodiscard]] bool requiresStoppedTargetState() const override {
            return true;
        }
    };
}
