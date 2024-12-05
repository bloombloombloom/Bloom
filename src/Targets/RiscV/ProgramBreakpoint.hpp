#pragma once

#include <optional>

#include "src/Targets/TargetBreakpoint.hpp"
#include "Opcodes/Opcode.hpp"

namespace Targets::RiscV
{
    struct ProgramBreakpoint: TargetProgramBreakpoint
    {
        std::optional<Opcodes::Opcode> originalInstruction = std::nullopt;

        explicit ProgramBreakpoint(const TargetProgramBreakpoint& breakpoint)
            : TargetProgramBreakpoint(breakpoint)
        {}

        explicit ProgramBreakpoint(const TargetProgramBreakpoint& breakpoint, Opcodes::Opcode originalInstruction)
            : TargetProgramBreakpoint(breakpoint)
            , originalInstruction(originalInstruction)
        {}
    };
}
