#pragma once

#include "Command.hpp"

#include "src/Targets/TargetRegister.hpp"

namespace Bloom::TargetController::Commands
{
    class WriteTargetRegisters: public Command
    {
    public:
        static constexpr CommandType type = CommandType::WRITE_TARGET_REGISTERS;
        static inline const std::string name = "WriteTargetRegisters";

        Targets::TargetRegisters registers;

        explicit WriteTargetRegisters(const Targets::TargetRegisters& registers)
            : registers(registers)
        {};

        [[nodiscard]] CommandType getType() const override {
            return WriteTargetRegisters::type;
        }
    };
}
