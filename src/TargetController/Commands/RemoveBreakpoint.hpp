#pragma once

#include "Command.hpp"

#include "src/Targets/TargetBreakpoint.hpp"

namespace Bloom::TargetController::Commands
{
    class RemoveBreakpoint: public Command
    {
    public:
        static constexpr CommandType type = CommandType::REMOVE_BREAKPOINT;
        static inline const std::string name = "RemoveBreakpoint";

        Targets::TargetBreakpoint breakpoint;

        RemoveBreakpoint() = default;
        explicit RemoveBreakpoint(const Targets::TargetBreakpoint& breakpoint)
            : breakpoint(breakpoint)
        {};

        [[nodiscard]] CommandType getType() const override {
            return RemoveBreakpoint::type;
        }

        [[nodiscard]] bool requiresStoppedTargetState() const override {
            return true;
        }
    };
}
