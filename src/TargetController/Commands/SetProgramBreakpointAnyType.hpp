#pragma once

#include "Command.hpp"
#include "src/TargetController/Responses/ProgramBreakpoint.hpp"

#include "src/Targets/TargetAddressSpaceDescriptor.hpp"
#include "src/Targets/TargetMemorySegmentDescriptor.hpp"
#include "src/Targets/TargetMemory.hpp"

namespace TargetController::Commands
{
    class SetProgramBreakpointAnyType: public Command
    {
    public:
        using SuccessResponseType = Responses::ProgramBreakpoint;

        static constexpr CommandType type = CommandType::SET_BREAKPOINT_ANY_TYPE;
        static const inline std::string name = "SetProgramBreakpointAnyType";

        const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor;
        const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor;
        Targets::TargetMemoryAddress address;
        Targets::TargetMemorySize size;

        SetProgramBreakpointAnyType(
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            Targets::TargetMemoryAddress address,
            Targets::TargetMemorySize size
        )
            : addressSpaceDescriptor(addressSpaceDescriptor)
            , memorySegmentDescriptor(memorySegmentDescriptor)
            , address(address)
            , size(size)
        {};

        [[nodiscard]] CommandType getType() const override {
            return SetProgramBreakpointAnyType::type;
        }

        [[nodiscard]] bool requiresStoppedTargetState() const override {
            return true;
        }
    };
}
