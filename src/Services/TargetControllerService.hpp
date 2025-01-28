#pragma once

#include <cstdint>
#include <chrono>
#include <memory>
#include <optional>
#include <functional>

#include "src/TargetController/CommandManager.hpp"
#include "src/TargetController/AtomicSession.hpp"

#include "src/Targets/TargetState.hpp"
#include "src/Targets/TargetAddressSpaceDescriptor.hpp"
#include "src/Targets/TargetMemorySegmentDescriptor.hpp"
#include "src/Targets/TargetPeripheralDescriptor.hpp"
#include "src/Targets/TargetRegisterGroupDescriptor.hpp"
#include "src/Targets/TargetRegisterDescriptor.hpp"
#include "src/Targets/TargetPinoutDescriptor.hpp"
#include "src/Targets/TargetPadDescriptor.hpp"
#include "src/Targets/TargetGpioPadState.hpp"
#include "src/Targets/TargetMemory.hpp"
#include "src/Targets/TargetMemoryAddressRange.hpp"
#include "src/Targets/TargetBreakpoint.hpp"
#include "src/Targets/PassthroughCommand.hpp"
#include "src/Targets/PassthroughResponse.hpp"

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
        [[nodiscard]] const Targets::TargetDescriptor& getTargetDescriptor() const;

        /**
         * Fetches the current target state.
         *
         * @return
         */
        [[nodiscard]] const Targets::TargetState& getTargetState() const;

        /**
         * Requests the TargetController to halt execution on the target.
         */
        void stopTargetExecution() const;

        /**
         * Requests the TargetController to resume execution on the target.
         */
        void resumeTargetExecution() const;

        /**
         * Requests the TargetController to step execution on the target.
         */
        void stepTargetExecution() const;

        /**
         * Requests the TargetController to read register values from the target.
         *
         * @param descriptors
         *  Descriptors of the registers to read.
         *
         * @return
         */
        [[nodiscard]] Targets::TargetRegisterDescriptorAndValuePairs readRegisters(
            const Targets::TargetRegisterDescriptors& descriptors
        ) const;

        /**
         * Requests the TargetController to read a single register value from the target.
         *
         * @param descriptor
         * @return
         */
        [[nodiscard]] Targets::TargetMemoryBuffer readRegister(const Targets::TargetRegisterDescriptor& descriptor) const;

        /**
         * Requests the TargetController to write register values to the target.
         *
         * @param registers
         */
        void writeRegisters(const Targets::TargetRegisterDescriptorAndValuePairs& registers) const;

        /**
         * Requests the TargetController to write to a single register on the target.
         *
         * @param descriptor
         * @param value
         */
        void writeRegister(
            const Targets::TargetRegisterDescriptor& descriptor,
            const Targets::TargetMemoryBuffer& value
        ) const;

        /**
         * Requests the TargetController to read memory from the target.
         *
         * @param addressSpaceDescriptor
         * @param memorySegmentDescriptor
         * @param startAddress
         * @param bytes
         * @param bypassCache
         * @param excludedAddressRanges
         * @return
         */
        [[nodiscard]] Targets::TargetMemoryBuffer readMemory(
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            Targets::TargetMemoryAddress startAddress,
            Targets::TargetMemorySize bytes,
            bool bypassCache = false,
            const std::set<Targets::TargetMemoryAddressRange>& excludedAddressRanges = {}
        ) const;

        /**
         * Requests the TargetController to write memory to the target.
         *
         * @param addressSpaceDescriptor
         * @param memorySegmentDescriptor
         * @param startAddress
         * @param buffer
         */
        void writeMemory(
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            Targets::TargetMemoryAddress startAddress,
            Targets::TargetMemoryBufferSpan buffer
        ) const;

        /**
         * Requests the TargetController to erase the given target memory segment.
         *
         * @param addressSpaceDescriptor
         * @param memorySegmentDescriptor
         */
        void eraseMemory(
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor
        ) const;

        /**
         * Requests the TargetController to set a program breakpoint of any type (hardware/software) on the target.
         *
         * @param addressSpaceDescriptor
         * @param memorySegmentDescriptor
         * @param address
         * @param size
         *
         * @return
         *  The installed breakpoint.
         */
        [[nodiscard]] Targets::TargetProgramBreakpoint setProgramBreakpointAnyType(
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            Targets::TargetMemoryAddress address,
            Targets::TargetMemorySize size
        ) const;

        /**
         * Requests the TargetController to remove a program breakpoint from the target.
         *
         * @param breakpoint
         */
        void removeProgramBreakpoint(const Targets::TargetProgramBreakpoint& breakpoint) const;

        /**
         * Retrieves the current program counter value from the target.
         *
         * @return
         */
        [[nodiscard]] Targets::TargetMemoryAddress getProgramCounter() const;

        /**
         * Sets the target's program counter to the given address.
         *
         * @param address
         */
        void setProgramCounter(Targets::TargetMemoryAddress address) const;

        /**
         * Retrieves the GPIO states for the given GPIO pads.
         *
         * @param padDescriptors
         */
        [[nodiscard]] Targets::TargetGpioPadDescriptorAndStatePairs getGpioPadStates(
            const Targets::TargetPadDescriptors& padDescriptors
        ) const;

        /**
         * Updates the pin state on the target, for a specific pin.
         *
         * @param padDescriptor
         * @param state
         */
        void setGpioPadState(
            const Targets::TargetPadDescriptor& padDescriptor,
            const Targets::TargetGpioPadState& state
        ) const;

        /**
         * Retrieves the current stack pointer value from the target.
         *
         * @return
         */
        [[nodiscard]] Targets::TargetStackPointer getStackPointer() const;

        /**
         * Sets the target's stack pointer to the given value.
         *
         * @param stackPointer
         */
        void setStackPointer(Targets::TargetStackPointer stackPointer) const;

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

        std::string getTargetPassthroughHelpText() const;

        std::optional<Targets::PassthroughResponse> invokeTargetPassthroughCommand(
            Targets::PassthroughCommand&& command
        ) const;

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

        std::chrono::milliseconds defaultTimeout = std::chrono::milliseconds{30000};

        TargetController::AtomicSessionIdType startAtomicSession();
        void endAtomicSession(TargetController::AtomicSessionIdType sessionId);
    };
}
