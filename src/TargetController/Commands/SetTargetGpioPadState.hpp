#pragma once

#include "Command.hpp"

#include "src/Targets/TargetPadDescriptor.hpp"
#include "src/Targets/TargetGpioPadState.hpp"

namespace TargetController::Commands
{
    class SetTargetGpioPadState: public Command
    {
    public:
        static constexpr CommandType type = CommandType::SET_TARGET_GPIO_PAD_STATE;
        static const inline std::string name = "SetTargetGpioPadState";

        const Targets::TargetPadDescriptor& padDescriptor;
        Targets::TargetGpioPadState state;

        SetTargetGpioPadState(
            const Targets::TargetPadDescriptor& padDescriptor,
            const Targets::TargetGpioPadState& state
        )
            : padDescriptor(padDescriptor)
            , state(state)
        {};

        [[nodiscard]] CommandType getType() const override {
            return SetTargetGpioPadState::type;
        }

        [[nodiscard]] bool requiresStoppedTargetState() const override {
            return true;
        }
    };
}
