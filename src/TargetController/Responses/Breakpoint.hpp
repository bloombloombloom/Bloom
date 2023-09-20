#pragma once

#include "Response.hpp"

#include "src/Targets/TargetBreakpoint.hpp"

namespace TargetController::Responses
{
    class Breakpoint: public Response
    {
    public:
        static constexpr ResponseType type = ResponseType::BREAKPOINT;

        Targets::TargetBreakpoint breakpoint;

        explicit Breakpoint(const Targets::TargetBreakpoint& breakpoint)
            : breakpoint(breakpoint)
        {}

        [[nodiscard]] ResponseType getType() const override {
            return Breakpoint::type;
        }
    };
}
