#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <set>
#include <map>

#include "src/ProjectConfig.hpp"

#include "TargetDescriptor.hpp"
#include "TargetState.hpp"
#include "TargetRegister.hpp"
#include "TargetMemory.hpp"
#include "TargetBreakpoint.hpp"

#include "src/DebugToolDrivers/DebugTool.hpp"

namespace Targets
{
    /**
     * Abstract class for Targets.
     *
     * All targets supported by Bloom must implement this interface.
     *
     * A single implementation of this interface can represent a single target, or an entire family of targets.
     */
    class Target
    {
    public:
        explicit Target() = default;

        virtual ~Target() = default;

        bool isActivated() const {
            return this->activated;
        }

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
         * Assuming the Target::isDebugToolSupported() check passed, this function will be called shortly after, by the
         * TargetController.
         *
         * @param debugTool
         */
        virtual void setDebugTool(DebugTool* debugTool) = 0;

        /**
         * This function should attempt to establish a connection with the target, and put it in a state where
         * debugging can be performed.
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
         * Should generate and return a TargetDescriptor for the current target.
         *
         * This is called when a component within Bloom requests the TargetDescriptor from the TargetController.
         * The TargetController will cache this upon the first request. Subsequent requests will be serviced with the
         * cached value.
         *
         * @return
         */
        virtual TargetDescriptor getDescriptor() = 0;

        /**
         * Should resume execution on the target.
         *
         * @param toAddress
         */
        virtual void run(std::optional<TargetMemoryAddress> toAddress = std::nullopt) = 0;

        /**
         * Should halt execution on the target.
         */
        virtual void stop() = 0;

        /**
         * Should step execution on the target (instruction step).
         */
        virtual void step() = 0;

        /**
         * Should reset the target.
         */
        virtual void reset() = 0;

        /**
         * Should set a software breakpoint on the target, at the given address.
         *
         * @param address
         */
        virtual void setSoftwareBreakpoint(TargetMemoryAddress address) = 0;

        /**
         * Should remove a software breakpoint at the given address.
         *
         * @param address
         */
        virtual void removeSoftwareBreakpoint(TargetMemoryAddress address) = 0;

        /**
         * Should set a hardware breakpoint on the target, at the given address.
         *
         * @param address
         */
        virtual void setHardwareBreakpoint(TargetMemoryAddress address) = 0;

        /**
         * Should remove a hardware breakpoint at the given address.
         *
         * @param address
         */
        virtual void removeHardwareBreakpoint(TargetMemoryAddress address) = 0;

        /**
         * Should clear all breakpoints on the target.
         *
         * @TODO: is this still needed? Review
         */
        virtual void clearAllBreakpoints() = 0;

        /**
         * Should read register values of the registers described by the given descriptors.
         *
         * @param descriptorIds
         *
         * @return
         */
        virtual TargetRegisters readRegisters(const Targets::TargetRegisterDescriptorIds& descriptorIds) = 0;

        /**
         * Should update the value of the given registers.
         *
         * @param registers
         */
        virtual void writeRegisters(const TargetRegisters& registers) = 0;

        /**
         * Should read memory from the target.
         *
         * @param memoryType
         * @param startAddress
         * @param bytes
         * @param excludedAddressRanges
         *
         * @return
         */
        virtual TargetMemoryBuffer readMemory(
            TargetMemoryType memoryType,
            TargetMemoryAddress startAddress,
            TargetMemorySize bytes,
            const std::set<Targets::TargetMemoryAddressRange>& excludedAddressRanges = {}
        ) = 0;

        /**
         * Should write memory to the target.
         *
         * @param memoryType
         * @param startAddress
         * @param buffer
         */
        virtual void writeMemory(
            TargetMemoryType memoryType,
            TargetMemoryAddress startAddress,
            const TargetMemoryBuffer& buffer
        ) = 0;

        /**
         * Should erase the entire address range of a given memory type.
         *
         * @param memoryType
         */
        virtual void eraseMemory(TargetMemoryType memoryType) = 0;

        /**
         * Should return the current state of the target.
         *
         * @return
         */
        virtual TargetState getState() = 0;

        /**
         * Should fetch the current program counter value.
         *
         * @return
         */
        virtual TargetMemoryAddress getProgramCounter() = 0;

        /**
         * Should update the program counter on the target.
         *
         * @param programCounter
         */
        virtual void setProgramCounter(TargetMemoryAddress programCounter) = 0;

        /**
         * Should fetch the current stack pointer value.
         *
         * @return
         */
        virtual TargetStackPointer getStackPointer() = 0;

        /**
         * Should get the current pin states for each pin on the target, mapped by pin number
         *
         * @param variantId
         *
         * @return
         */
        virtual std::map<int, TargetPinState> getPinStates(int variantId) = 0;

        /**
         * Should update the pin state for the given pin, with the given state.
         *
         * @param pinDescriptor
         * @param state
         */
        virtual void setPinState(
            const TargetPinDescriptor& pinDescriptor,
            const TargetPinState& state
        ) = 0;

        /**
         * Should prepare the target for programming.
         */
        virtual void enableProgrammingMode() = 0;

        /**
         * Should prepare the target for resuming debugging operations after programming.
         */
        virtual void disableProgrammingMode() = 0;

        /**
         * Should return true if programming is currently enabled. Otherwise false.
         *
         * @return
         */
        virtual bool programmingModeEnabled() = 0;

    protected:
        /**
         * Target related configuration provided by the user. This is passed in via the first stage of target
         * configuration. See Target::preActivationConfigure() for more.
         */
        TargetConfig config;

        bool activated = false;
    };
}
