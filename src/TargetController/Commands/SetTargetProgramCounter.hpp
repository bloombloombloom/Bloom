#pragma once

#include <cstdint>

#include "Command.hpp"

#include "src/Targets/TargetBreakpoint.hpp"

namespace Bloom::TargetController::Commands
{
    class SetTargetProgramCounter: public Command
    {
    public:
        static constexpr CommandType type = CommandType::SET_TARGET_PROGRAM_COUNTER;
        static inline const std::string name = "SetTargetProgramCounter";

        std::uint32_t address = 0;

        explicit SetTargetProgramCounter(std::uint32_t address)
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
