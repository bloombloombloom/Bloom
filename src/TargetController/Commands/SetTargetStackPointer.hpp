#pragma once

#include <cstdint>

#include "Command.hpp"

#include "src/Targets/TargetMemory.hpp"

namespace TargetController::Commands
{
    class SetTargetStackPointer: public Command
    {
    public:
        static constexpr CommandType type = CommandType::SET_TARGET_STACK_POINTER;
        static const inline std::string name = "SetTargetStackPointer";

        Targets::TargetStackPointer stackPointer;

        explicit SetTargetStackPointer(Targets::TargetStackPointer stackPointer)
            : stackPointer(stackPointer)
        {};

        [[nodiscard]] CommandType getType() const override {
            return SetTargetStackPointer::type;
        }

        [[nodiscard]] bool requiresStoppedTargetState() const override {
            return true;
        }
    };
}
