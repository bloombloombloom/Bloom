#pragma once

#include <cstdint>
#include <chrono>
#include <optional>
#include <functional>

#include "src/TargetController/CommandManager.hpp"
#include "src/TargetController/AtomicSession.hpp"

#include "src/Targets/TargetState.hpp"
#include "src/Targets/TargetRegister.hpp"
#include "src/Targets/TargetMemory.hpp"
#include "src/Targets/TargetBreakpoint.hpp"
#include "src/Targets/TargetVariant.hpp"
#include "src/Targets/TargetPinDescriptor.hpp"

#include "src/Exceptions/Exception.hpp"

namespace Services
{
    /**
     * The TargetControllerService provides an interface to the TargetController.
     */
    class TargetControllerService
    {
    public:
        /**
         * RAII wrapper for atomic sessions.
         */
        class AtomicSession
        {
        public:
            explicit AtomicSession(TargetControllerService& targetControllerService);
            ~AtomicSession();

            AtomicSession(const AtomicSession&) = delete;
            AtomicSession(const AtomicSession&&) = delete;
            AtomicSession& operator = (const AtomicSession&) = delete;
            AtomicSession& operator = (const AtomicSession&&) = delete;

        private:
            TargetControllerService& targetControllerService;
            TargetController::AtomicSessionIdType sessionId;
        };

        TargetControllerService() = default;

        void setDefaultTimeout(std::chrono::milliseconds timeout) {
            this->defaultTimeout = timeout;
        }

        /**
         * Requests the TargetDescriptor from the TargetController
         *
         * @return
         */
        const Targets::TargetDescriptor& getTargetDescriptor() const;

        /**
         * Fetches the current target state.
         *
         * @return
         */
        Targets::TargetState getTargetState() const;

        /**
         * Requests the TargetController to halt execution on the target.
         */
        void stopTargetExecution() const;

        /**
         * Requests the TargetController to continue execution on the target.
         *
         * @param fromAddress
         */
        void continueTargetExecution(
            std::optional<Targets::TargetMemoryAddress> fromAddress,
            std::optional<Targets::TargetMemoryAddress> toAddress
        ) const;

        /**
         * Requests the TargetController to step execution on the target.
         *
         * @param fromAddress
         */
        void stepTargetExecution(std::optional<Targets::TargetMemoryAddress> fromAddress) const;

        /**
         * Requests the TargetController to read register values from the target.
         *
         * @param descriptorIds
         *  Descriptor IDs of the registers to read.
         *
         * @return
         */
        Targets::TargetRegisters readRegisters(const Targets::TargetRegisterDescriptorIds& descriptorIds) const;

        /**
         * Requests the TargetController to write register values to the target.
         *
         * @param registers
         */
        void writeRegisters(const Targets::TargetRegisters& registers) const;

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
            Targets::TargetMemoryAddress startAddress,
            Targets::TargetMemorySize bytes,
            const std::set<Targets::TargetMemoryAddressRange>& excludedAddressRanges = {}
        ) const;

        /**
         * Requests the TargetController to write memory to the target.
         *
         * @param memoryType
         * @param startAddress
         * @param buffer
         */
        void writeMemory(
            Targets::TargetMemoryType memoryType,
            Targets::TargetMemoryAddress startAddress,
            const Targets::TargetMemoryBuffer& buffer
        ) const;

        /**
         * Requests the TargetController to erase the given target memory type.
         *
         * @param memoryType
         */
        void eraseMemory(Targets::TargetMemoryType memoryType) const;

        /**
         * Requests the TargetController to set a breakpoint on the target.
         *
         * @param breakpoint
         */
        void setBreakpoint(Targets::TargetBreakpoint breakpoint) const;

        /**
         * Requests the TargetController to remove a breakpoint from the target.
         *
         * @param breakpoint
         */
        void removeBreakpoint(Targets::TargetBreakpoint breakpoint) const;

        /**
         * Retrieves the current program counter value from the target.
         *
         * @return
         */
        Targets::TargetProgramCounter getProgramCounter() const;

        /**
         * Sets the target's program counter to the given address.
         *
         * @param address
         */
        void setProgramCounter(Targets::TargetProgramCounter address) const;

        /**
         * Retrieves the pin states for a particular target variant.
         *
         * @param variantId
         */
        Targets::TargetPinStateMapping getPinStates(int variantId) const;

        /**
         * Updates the pin state on the target, for a specific pin.
         *
         * @param pinDescriptor
         * @param pinState
         */
        void setPinState(Targets::TargetPinDescriptor pinDescriptor, Targets::TargetPinState pinState) const;

        /**
         * Retrieves the current stack pointer value from the target.
         *
         * @return
         */
        Targets::TargetStackPointer getStackPointer() const;

        /**
         * Triggers a reset on the target. The target will be held in a stopped state.
         */
        void resetTarget() const;

        /**
         * Enables programming mode on the target.
         *
         * From the point of invoking this function, the TargetController will reject any subsequent commands for
         * debug operations (such as ResumeTargetExecution, ReadTargetRegisters, etc), until programming mode has
         * been disabled.
         */
        void enableProgrammingMode() const;

        /**
         * Disables programming mode on the target.
         */
        void disableProgrammingMode() const;

        /**
         * Forces the TargetController to shutdown
         */
        void shutdown() const;

        /**
         * Starts a new atomic session with the TC, via an TargetControllerService::AtomicSession RAII object.
         * The session will end when the object is destroyed.
         *
         * @return
         */
        TargetControllerService::AtomicSession makeAtomicSession();

    private:
        TargetController::CommandManager commandManager = TargetController::CommandManager();

        std::optional<TargetController::AtomicSessionIdType> activeAtomicSessionId = std::nullopt;

        std::chrono::milliseconds defaultTimeout = std::chrono::milliseconds(60000);

        TargetController::AtomicSessionIdType startAtomicSession();
        void endAtomicSession(TargetController::AtomicSessionIdType sessionId);
    };
}
