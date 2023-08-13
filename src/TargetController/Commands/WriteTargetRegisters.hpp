#pragma once

#include "Command.hpp"

#include "src/Targets/TargetRegister.hpp"

namespace TargetController::Commands
{
    class WriteTargetRegisters: public Command
    {
    public:
        static constexpr CommandType type = CommandType::WRITE_TARGET_REGISTERS;
        static const inline std::string name = "WriteTargetRegisters";

        Targets::TargetRegisters registers;

        explicit WriteTargetRegisters(const Targets::TargetRegisters& registers)
            : registers(registers)
        {};

        [[nodiscard]] CommandType getType() const override {
            return WriteTargetRegisters::type;
        }

        [[nodiscard]] bool requiresStoppedTargetState() const override {
            return true;
        }
    };
}
