#pragma once

#include "Response.hpp"

#include "src/Targets/TargetBreakpoint.hpp"

namespace TargetController::Responses
{
    class ProgramBreakpoint: public Response
    {
    public:
        static constexpr ResponseType type = ResponseType::PROGRAM_BREAKPOINT;

        Targets::TargetProgramBreakpoint breakpoint;

        explicit ProgramBreakpoint(const Targets::TargetProgramBreakpoint& breakpoint)
            : breakpoint(breakpoint)
        {}

        [[nodiscard]] ResponseType getType() const override {
            return ProgramBreakpoint::type;
        }
    };
}
