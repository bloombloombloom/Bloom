#pragma once

#include "Command.hpp"

#include "src/Targets/TargetPinDescriptor.hpp"

namespace Bloom::TargetController::Commands
{
    class SetTargetPinState: public Command
    {
    public:
        static constexpr CommandType type = CommandType::SET_TARGET_PIN_STATE;
        static const inline std::string name = "SetTargetPinState";

        Targets::TargetPinDescriptor pinDescriptor;
        Targets::TargetPinState pinState;

        SetTargetPinState(
            const Targets::TargetPinDescriptor& pinDescriptor,
            const Targets::TargetPinState& pinState
        )
            : pinDescriptor(pinDescriptor)
            , pinState(pinState)
        {};

        [[nodiscard]] CommandType getType() const override {
            return SetTargetPinState::type;
        }

        [[nodiscard]] bool requiresStoppedTargetState() const override {
            return true;
        }
    };
}
