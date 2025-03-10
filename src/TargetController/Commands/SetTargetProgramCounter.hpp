#pragma once

#include <cstdint>

#include "Command.hpp"

#include "src/Targets/TargetMemory.hpp"

namespace TargetController::Commands
{
    class SetTargetProgramCounter: public Command
    {
    public:
        static constexpr CommandType type = CommandType::SET_TARGET_PROGRAM_COUNTER;
        static const inline std::string name = "SetTargetProgramCounter";

        Targets::TargetMemoryAddress address;

        explicit SetTargetProgramCounter(Targets::TargetMemoryAddress address)
            : address(address)
        {};

        [[nodiscard]] CommandType getType() const override {
            return SetTargetProgramCounter::type;
        }

        [[nodiscard]] bool requiresStoppedTargetState() const override {
            return true;
        }
    };
}
