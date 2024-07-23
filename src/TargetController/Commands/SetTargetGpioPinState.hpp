#pragma once

#include "Command.hpp"

#include "src/Targets/TargetPinDescriptor.hpp"
#include "src/Targets/TargetGpioPinState.hpp"

namespace TargetController::Commands
{
    class SetTargetGpioPinState: public Command
    {
    public:
        static constexpr CommandType type = CommandType::SET_TARGET_GPIO_PIN_STATE;
        static const inline std::string name = "SetTargetGpioPinState";

        const Targets::TargetPinDescriptor& pinDescriptor;
        Targets::TargetGpioPinState state;

        SetTargetGpioPinState(
            const Targets::TargetPinDescriptor& pinDescriptor,
            const Targets::TargetGpioPinState& state
        )
            : pinDescriptor(pinDescriptor)
            , state(state)
        {};

        [[nodiscard]] CommandType getType() const override {
            return SetTargetGpioPinState::type;
        }

        [[nodiscard]] bool requiresStoppedTargetState() const override {
            return true;
        }
    };
}
