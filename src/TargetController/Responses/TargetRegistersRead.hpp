#pragma once

#include "Response.hpp"

#include "src/Targets/TargetRegisterDescriptor.hpp"
#include "src/Targets/TargetMemory.hpp"

namespace TargetController::Responses
{
    class TargetRegistersRead: public Response
    {
    public:
        static constexpr ResponseType type = ResponseType::TARGET_REGISTERS_READ;

        Targets::TargetRegisterDescriptorAndValuePairs registers;

        explicit TargetRegistersRead(
            Targets::TargetRegisterDescriptorAndValuePairs&& registers
        )
            : registers(std::move(registers))
        {}

        [[nodiscard]] ResponseType getType() const override {
            return TargetRegistersRead::type;
        }
    };
}
