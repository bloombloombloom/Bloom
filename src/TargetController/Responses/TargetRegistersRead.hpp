#pragma once

#include "Response.hpp"

#include "src/Targets/TargetRegister.hpp"

namespace TargetController::Responses
{
    class TargetRegistersRead: public Response
    {
    public:
        static constexpr ResponseType type = ResponseType::TARGET_REGISTERS_READ;

        Targets::TargetRegisters registers;

        explicit TargetRegistersRead(const Targets::TargetRegisters& registers)
            : registers(registers)
        {}

        [[nodiscard]] ResponseType getType() const override {
            return TargetRegistersRead::type;
        }
    };
}
