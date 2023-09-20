#pragma once

#include "Command.hpp"
#include "src/TargetController/Responses/Breakpoint.hpp"

#include "src/Targets/TargetBreakpoint.hpp"
#include "src/Targets/TargetMemory.hpp"

namespace TargetController::Commands
{
    class SetBreakpoint: public Command
    {
    public:
        using SuccessResponseType = Responses::Breakpoint;

        static constexpr CommandType type = CommandType::SET_BREAKPOINT;
        static const inline std::string name = "SetBreakpoint";

        /**
         * Byte address in program memory.
         */
        Targets::TargetMemoryAddress address;

        /**
         * The preferred breakpoint type (HARDWARE/SOFTWARE).
         *
         * There is no guarantee that the TC will be able to allocate resources for the preferred type.
         * If the preferredType is set to HARDWARE, but the target doesn't have any available resources, or hardware
         * breakpoints have been disabled (@see TargetConfig::hardwareBreakpoints), the TC will fall back to software
         * breakpoints.
         */
        Targets::TargetBreakpoint::Type preferredType;

        SetBreakpoint(Targets::TargetMemoryAddress address, Targets::TargetBreakpoint::Type preferredType)
            : address(address)
            , preferredType(preferredType)
        {};

        [[nodiscard]] CommandType getType() const override {
            return SetBreakpoint::type;
        }

        [[nodiscard]] bool requiresStoppedTargetState() const override {
            return true;
        }
    };
}
