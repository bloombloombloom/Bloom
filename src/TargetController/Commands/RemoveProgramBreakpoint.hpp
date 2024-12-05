#pragma once

#include "Command.hpp"

#include "src/Targets/TargetBreakpoint.hpp"

namespace TargetController::Commands
{
    class RemoveProgramBreakpoint: public Command
    {
    public:
        static constexpr CommandType type = CommandType::REMOVE_PROGRAM_BREAKPOINT;
        static const inline std::string name = "RemoveProgramBreakpoint";

        Targets::TargetProgramBreakpoint breakpoint;

        explicit RemoveProgramBreakpoint(const Targets::TargetProgramBreakpoint& breakpoint)
            : breakpoint(breakpoint)
        {};

        [[nodiscard]] CommandType getType() const override {
            return RemoveProgramBreakpoint::type;
        }

        [[nodiscard]] bool requiresStoppedTargetState() const override {
            return true;
        }
    };
}
