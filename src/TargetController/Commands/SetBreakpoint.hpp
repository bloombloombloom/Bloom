#pragma once

#include "Command.hpp"

#include "src/Targets/TargetBreakpoint.hpp"

namespace Bloom::TargetController::Commands
{
    class SetBreakpoint: public Command
    {
    public:
        static constexpr CommandType type = CommandType::SET_BREAKPOINT;
        static const inline std::string name = "SetBreakpoint";

        Targets::TargetBreakpoint breakpoint;

        SetBreakpoint() = default;
        explicit SetBreakpoint(const Targets::TargetBreakpoint& breakpoint)
            : breakpoint(breakpoint)
        {};

        [[nodiscard]] CommandType getType() const override {
            return SetBreakpoint::type;
        }

        [[nodiscard]] bool requiresStoppedTargetState() const override {
            return true;
        }
    };
}
