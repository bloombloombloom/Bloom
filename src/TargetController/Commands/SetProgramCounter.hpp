#pragma once

#include <cstdint>

#include "Command.hpp"

#include "src/Targets/TargetBreakpoint.hpp"

namespace Bloom::TargetController::Commands
{
    class SetProgramCounter: public Command
    {
    public:
        static constexpr CommandType type = CommandType::SET_PROGRAM_COUNTER;
        static inline const std::string name = "SetProgramCounter";

        std::uint32_t address = 0;

        SetProgramCounter() = default;
        explicit SetProgramCounter(std::uint32_t address)
            : address(address)
        {};

        [[nodiscard]] CommandType getType() const override {
            return SetProgramCounter::type;
        }

        [[nodiscard]] bool requiresStoppedTargetState() const override {
            return true;
        }
    };
}
