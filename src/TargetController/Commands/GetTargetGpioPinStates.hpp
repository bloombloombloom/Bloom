#pragma once

#include "Command.hpp"

#include "src/TargetController/Responses/TargetGpioPinStates.hpp"

namespace TargetController::Commands
{
    class GetTargetGpioPinStates: public Command
    {
    public:
        using SuccessResponseType = Responses::TargetGpioPinStates;

        static constexpr CommandType type = CommandType::GET_TARGET_GPIO_PIN_STATES;
        static const inline std::string name = "GetTargetGpioPinStates";

        const Targets::TargetPinoutDescriptor& pinoutDescriptor;

        explicit GetTargetGpioPinStates(const Targets::TargetPinoutDescriptor& pinoutDescriptor)
            : pinoutDescriptor(pinoutDescriptor)
        {};

        [[nodiscard]] CommandType getType() const override {
            return GetTargetGpioPinStates::type;
        }

        [[nodiscard]] bool requiresStoppedTargetState() const override {
            return true;
        }
    };
}
