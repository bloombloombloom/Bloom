#pragma once

#include <cstdint>
#include <optional>

#include "CommandManager.hpp"
#include "TargetControllerState.hpp"

#include "src/Targets/TargetState.hpp"
#include "src/Targets/TargetRegister.hpp"
#include "src/Targets/TargetMemory.hpp"
#include "src/Targets/TargetBreakpoint.hpp"
#include "src/Targets/TargetVariant.hpp"
#include "src/Targets/TargetPinDescriptor.hpp"

#include "src/Exceptions/Exception.hpp"

namespace Bloom::TargetController
{
    /**
     * The TargetControllerConsole provides an interface to the TargetController.
     */
    class TargetControllerConsole
    {
    public:
        TargetControllerConsole() = default;

        void setDefaultTimeout(std::chrono::milliseconds timeout) {
            this->defaultTimeout = timeout;
        }

        /**
         * Requests the current TargetController state from the TargetController. The TargetController should always
         * respond to such a request, even when it's in a suspended state.
         *
         * To check if the TargetController is in an active state, isTargetControllerInService() can be used for
         * convenience.
         *
         * @return
         */
        TargetControllerState getTargetControllerState();

        /**
         * Retrieves the TargetController state and checks if it's currently active.
         *
         * @return
         *  True if the TargetController is currently in an active state, otherwise false.
         */
        bool isTargetControllerInService() noexcept;

        /**
         * Requests the TargetDescriptor from the TargetController
         *
         * @return
         */
        Targets::TargetDescriptor getTargetDescriptor();

        /**
         * Fetches the current target state.
         *
         * @return
         */
        Targets::TargetState getTargetState();

        /**
         * Requests the TargetController to halt execution on the target.
         */
        void stopTargetExecution();

        /**
         * Requests the TargetController to continue execution on the target.
         *
         * @param fromAddress
         */
        void continueTargetExecution(std::optional<std::uint32_t> fromAddress);

        /**
         * Requests the TargetController to step execution on the target.
         *
         * @param fromAddress
         */
        void stepTargetExecution(std::optional<std::uint32_t> fromAddress);

        /**
         * Requests the TargetController to read register values from the target.
         *
         * @param descriptors
         *  Descriptors of the registers to read.
         *
         * @return
         */
        Targets::TargetRegisters readRegisters(const Targets::TargetRegisterDescriptors& descriptors);

        /**
         * Requests the TargetController to write register values to the target.
         *
         * @param registers
         */
        void writeRegisters(const Targets::TargetRegisters& registers);

        /**
         * Requests the TargetController to read memory from the target.
         *
         * @param memoryType
         * @param startAddress
         * @param bytes
         * @param excludedAddressRanges
         * @return
         */
        Targets::TargetMemoryBuffer readMemory(
            Targets::TargetMemoryType memoryType,
            std::uint32_t startAddress,
            std::uint32_t bytes,
            const std::set<Targets::TargetMemoryAddressRange>& excludedAddressRanges = {}
        );

        /**
         * Requests the TargetController to write memory to the target.
         *
         * @param memoryType
         * @param startAddress
         * @param buffer
         */
        void writeMemory(
            Targets::TargetMemoryType memoryType,
            std::uint32_t startAddress,
            const Targets::TargetMemoryBuffer& buffer
        );

        /**
         * Requests the TargetController to set a breakpoint on the target.
         *
         * @param breakpoint
         */
        void setBreakpoint(Targets::TargetBreakpoint breakpoint);

        /**
         * Requests the TargetController to remove a breakpoint from the target.
         *
         * @param breakpoint
         */
        void removeBreakpoint(Targets::TargetBreakpoint breakpoint);

        /**
         * Retrieves the current program counter value from the target.
         *
         * @return
         */
        std::uint32_t getProgramCounter();

        /**
         * Sets the target's program counter to the given address.
         *
         * @param address
         */
        void setProgramCounter(std::uint32_t address);

        /**
         * Retrieves the pin states for a particular target variant.
         *
         * @param variantId
         */
        Targets::TargetPinStateMappingType getPinStates(int variantId);

        /**
         * Updates the pin state on the target, for a specific pin.
         *
         * @param pinDescriptor
         * @param pinState
         */
        void setPinState(Targets::TargetPinDescriptor pinDescriptor, Targets::TargetPinState pinState);

        /**
         * Retrieves the current stack pointer value from the target.
         *
         * @return
         */
        std::uint32_t getStackPointer();

        /**
         * Triggers a reset on the target. The target will be held in a stopped state.
         */
        void resetTarget();

    private:
        CommandManager commandManager = CommandManager();

        std::chrono::milliseconds defaultTimeout = std::chrono::milliseconds(20000);
    };
}
