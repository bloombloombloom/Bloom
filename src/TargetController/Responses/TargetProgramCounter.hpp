#pragma once

#include <cstdint>

#include "Response.hpp"

namespace Bloom::TargetController::Responses
{
    class TargetProgramCounter: public Response
    {
    public:
        static constexpr ResponseType type = ResponseType::TARGET_PROGRAM_COUNTER;

        std::uint32_t programCounter;

        explicit TargetProgramCounter(std::uint32_t programCounter)
            : programCounter(programCounter)
        {}

        [[nodiscard]] ResponseType getType() const override {
            return TargetProgramCounter::type;
        }
    };
}
