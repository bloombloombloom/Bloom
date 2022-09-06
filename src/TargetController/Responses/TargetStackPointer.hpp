#pragma once

#include <cstdint>

#include "Response.hpp"

#include "src/Targets/TargetMemory.hpp"

namespace Bloom::TargetController::Responses
{
    class TargetStackPointer: public Response
    {
    public:
        static constexpr ResponseType type = ResponseType::TARGET_STACK_POINTER;

        Targets::TargetStackPointer stackPointer;

        explicit TargetStackPointer(Targets::TargetStackPointer stackPointer)
            : stackPointer(stackPointer)
        {}

        [[nodiscard]] ResponseType getType() const override {
            return TargetStackPointer::type;
        }
    };
}
