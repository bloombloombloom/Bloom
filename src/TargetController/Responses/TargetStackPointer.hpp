#pragma once

#include <cstdint>

#include "Response.hpp"

namespace Bloom::TargetController::Responses
{
    class TargetStackPointer: public Response
    {
    public:
        static constexpr ResponseType type = ResponseType::TARGET_STACK_POINTER;

        std::uint32_t stackPointer;

        explicit TargetStackPointer(std::uint32_t stackPointer)
            : stackPointer(stackPointer)
        {}

        [[nodiscard]] ResponseType getType() const override {
            return TargetStackPointer::type;
        }
    };
}
