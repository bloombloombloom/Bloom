#pragma once

#include <cstdint>
#include <optional>

#include "src/EventManager/EventListener.hpp"
#include "src/EventManager/EventManager.hpp"

#include "src/Targets/TargetRegister.hpp"
#include "src/Targets/TargetMemory.hpp"
#include "src/Targets/TargetBreakpoint.hpp"
#include "src/Targets/TargetVariant.hpp"
#include "src/Targets/TargetState.hpp"
#include "src/Targets/TargetPinDescriptor.hpp"

namespace Bloom
{
    using namespace Targets;

    /**
     * The TargetControllerConsole provides an interface to the TargetController, for components within Bloom that
     * require access to common functionality from the TargetController.
     */
    class TargetControllerConsole
    {
    private:
        EventManager& eventManager;
        EventListener& eventListener;

        std::chrono::milliseconds defaultTimeout = std::chrono::milliseconds(10000);

    public:
        TargetControllerConsole(EventManager& eventManager, EventListener& eventListener):
        eventManager(eventManager), eventListener(eventListener) {};

        void setDefaultTimeout(std::chrono::milliseconds timeout) {
            this->defaultTimeout = timeout;
        }

        /**
         * Requests the TargetDescriptor from the TargetController
         *
         * @return
         */
        TargetDescriptor getTargetDescriptor();

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
        TargetRegisters readGeneralRegisters(TargetRegisterDescriptors descriptors);

        /**
         * Requests the TargetController to write register values to the target.
         *
         * @param registers
         */
        void writeGeneralRegisters(TargetRegisters registers);

        /**
         * Requests the TargetController to read memory from the target.
         *
         * @param memoryType
         * @param startAddress
         * @param bytes
         * @return
         */
        TargetMemoryBuffer readMemory(TargetMemoryType memoryType, std::uint32_t startAddress, std::uint32_t bytes);

        /**
         * Requests the TargetController to write memory to the target.
         *
         * @param memoryType
         * @param startAddress
         * @param buffer
         */
        void writeMemory(TargetMemoryType memoryType, std::uint32_t startAddress, const TargetMemoryBuffer& buffer);

        /**
         * Requests the TargetController to set a breakpoint on the target.
         *
         * @param breakpoint
         */
        void setBreakpoint(TargetBreakpoint breakpoint);

        /**
         * Requests the TargetController to remove a breakpoint from the target.
         *
         * @param breakpoint
         */
        void removeBreakpoint(TargetBreakpoint breakpoint);

        /**
         * Requests a pin state update on the target, for a specific pin.
         *
         * @param variantId
         * @param pinDescriptor
         * @param pinState
         */
        void setPinState(int variantId, TargetPinDescriptor pinDescriptor, TargetPinState pinState);

        /**
         * Requests a pin state refresh from the TargetController, for a specific target variant.
         *
         * @param variantId
         */
        void requestPinStates(int variantId);
    };
}
