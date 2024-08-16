#pragma once

#include "Command.hpp"

#include "src/TargetController/Responses/TargetGpioPadStates.hpp"
#include "src/Targets/TargetPadDescriptor.hpp"

namespace TargetController::Commands
{
    class GetTargetGpioPadStates: public Command
    {
    public:
        using SuccessResponseType = Responses::TargetGpioPadStates;

        static constexpr CommandType type = CommandType::GET_TARGET_GPIO_PAD_STATES;
        static const inline std::string name = "GetTargetGpioPadStates";

        const Targets::TargetPadDescriptors padDescriptors;

        explicit GetTargetGpioPadStates(const Targets::TargetPadDescriptors& padDescriptors)
            : padDescriptors(padDescriptors)
        {};

        [[nodiscard]] CommandType getType() const override {
            return GetTargetGpioPadStates::type;
        }

        [[nodiscard]] bool requiresStoppedTargetState() const override {
            return true;
        }
    };
}
