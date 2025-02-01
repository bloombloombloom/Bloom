#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <set>
#include <map>
#include <optional>

#include "src/ProjectConfig.hpp"

#include "TargetDescriptor.hpp"
#include "TargetAddressSpaceDescriptor.hpp"
#include "TargetMemorySegmentDescriptor.hpp"
#include "TargetState.hpp"
#include "TargetRegisterDescriptor.hpp"
#include "TargetMemory.hpp"
#include "TargetMemoryAddressRange.hpp"
#include "TargetBreakpoint.hpp"
#include "TargetPadDescriptor.hpp"
#include "TargetGpioPadState.hpp"

#include "PassthroughCommand.hpp"
#include "PassthroughResponse.hpp"

#include "DeltaProgramming/DeltaProgrammingInterface.hpp"

#include "src/DebugToolDrivers/DebugTool.hpp"

namespace Targets
{
    class Target
    {
    public:
        Target() = default;
        virtual ~Target() = default;

        /**
         * Should check if the given debugTool is compatible with the target. Returning false in this function will
         * prevent Bloom from attempting to use the selected debug tool with the selected target. An InvalidConfig
         * exception will be raised and Bloom will shutdown.
         *
         * @param debugTool
         *
         * @return
         */
        virtual bool supportsDebugTool(DebugTool* debugTool) = 0;

        /**
         * Assuming the Target::supportsDebugTool() check passed, this function will be called shortly after, by the
         * TargetController.
         *
         * @param debugTool
         */
        virtual void setDebugTool(DebugTool* debugTool) = 0;

        /**
         * This function should attempt to establish a connection with the target, halt execution, and put it in a
         * state where debugging can be performed.
         *
         * If an exception is thrown from this function, the TargetController will treat it as a fatal error, and thus
         * will shutdown, along with the rest of Bloom.
         */
        virtual void activate() = 0;

        /**
         * Should pull the target out of the debugging state and disconnect from it.
         *
         * This is typically called on TargetController shutdown, but keep in mind that it's called regardless of
         * whether or not Target::activate() was previously called. In other words, the TargetController will always
         * call this function on shutdown, even if the TargetController did not call Target::activate() before it began
         * shutting down. The reason behind this is to give the target a chance to deactivate in cases where the call
         * to Target::activate() failed and thus triggered a shutdown (via an exception being thrown from
         * Target::activate()).
         */
        virtual void deactivate() = 0;

        /**
         * This function is called immediately after successful target activation.
         *
         * It's a good place to log any info that we can extract from the target, that will be useful to the user.
         */
        virtual void postActivate() = 0;

        /**
         * Should generate a TargetDescriptor for the target.
         *
         * This function is called shortly after target activation, allowing for information that was obtained via
         * probing/discovery to be used in the target descriptor.
         *
         * @return
         */
        virtual TargetDescriptor targetDescriptor() = 0;

        virtual void run(std::optional<TargetMemoryAddress> toAddress) = 0;
        virtual void stop() = 0;
        virtual void step() = 0;
        virtual void reset() = 0;

        virtual void setProgramBreakpoint(const TargetProgramBreakpoint& breakpoint) = 0;
        virtual void removeProgramBreakpoint(const TargetProgramBreakpoint& breakpoint) = 0;

        virtual TargetRegisterDescriptorAndValuePairs readRegisters(const TargetRegisterDescriptors& descriptors) = 0;
        virtual void writeRegisters(const TargetRegisterDescriptorAndValuePairs& registers) = 0;

        virtual TargetMemoryBuffer readMemory(
            const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            TargetMemoryAddress startAddress,
            TargetMemorySize bytes,
            const std::set<TargetMemoryAddressRange>& excludedAddressRanges
        ) = 0;
        virtual void writeMemory(
            const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            TargetMemoryAddress startAddress,
            TargetMemoryBufferSpan buffer
        ) = 0;

        /**
         * Should check if the given memory is program memory.
         *
         * The TargetMemorySegmentDescriptor::executable flag specifies whether any part of the segment is executable,
         * but this member function allows for a more granular check.
         *
         * @param addressSpaceDescriptor
         * @param memorySegmentDescriptor
         * @param startAddress
         * @param size
         *
         * @return
         */
        virtual bool isProgramMemory(
            const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            TargetMemoryAddress startAddress,
            TargetMemorySize size
        ) = 0;

        virtual void eraseMemory(
            const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const TargetMemorySegmentDescriptor& memorySegmentDescriptor
        ) = 0;

        virtual TargetExecutionState getExecutionState() = 0;

        virtual TargetMemoryAddress getProgramCounter() = 0;
        virtual void setProgramCounter(TargetMemoryAddress programCounter) = 0;

        virtual TargetStackPointer getStackPointer() = 0;
        virtual void setStackPointer(TargetStackPointer stackPointer) = 0;

        virtual TargetGpioPadDescriptorAndStatePairs getGpioPadStates(const TargetPadDescriptors& padDescriptors) = 0;
        virtual void setGpioPadState(const TargetPadDescriptor& padDescriptor, const TargetGpioPadState& state) = 0;

        /**
         * When enabling programming mode, the target driver is expected to clear all program breakpoints currently
         * installed on the target.
         *
         * Before disabling program mode, the target controller will reinstall the necessary breakpoints.
         */
        virtual void enableProgrammingMode() = 0;
        virtual void disableProgrammingMode() = 0;
        virtual bool programmingModeEnabled() = 0;

        virtual std::string passthroughCommandHelpText() = 0;
        virtual std::optional<PassthroughResponse> invokePassthroughCommand(const PassthroughCommand& command) = 0;

        virtual DeltaProgramming::DeltaProgrammingInterface* deltaProgrammingInterface() = 0;
    };
}
