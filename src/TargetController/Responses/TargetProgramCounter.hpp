#pragma once

#include <cstdint>

#include "Response.hpp"

#include "src/Targets/TargetMemory.hpp"

namespace TargetController::Responses
{
    class TargetProgramCounter: public Response
    {
    public:
        static constexpr ResponseType type = ResponseType::TARGET_PROGRAM_COUNTER;

        Targets::TargetMemoryAddress programCounter;

        explicit TargetProgramCounter(Targets::TargetMemoryAddress programCounter)
            : programCounter(programCounter)
        {}

        [[nodiscard]] ResponseType getType() const override {
            return TargetProgramCounter::type;
        }
    };
}
