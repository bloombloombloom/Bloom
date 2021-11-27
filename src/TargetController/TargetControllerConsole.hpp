#pragma once

#include <cstdint>
#include <optional>

#include "TargetControllerState.hpp"

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
    /**
     * The TargetControllerConsole provides an interface to the TargetController.
     */
    class TargetControllerConsole
    {
    public:
        TargetControllerConsole(EventManager& eventManager, EventListener& eventListener);

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
         * @return
         */
        Targets::TargetMemoryBuffer readMemory(
            Targets::TargetMemoryType memoryType,
            std::uint32_t startAddress,
            std::uint32_t bytes
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
         * Requests a pin state refresh from the TargetController, for a specific target variant.
         *
         * @param variantId
         */
        void requestPinStates(int variantId);

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

    private:
        EventManager& eventManager;
        EventListener& eventListener;

        std::chrono::milliseconds defaultTimeout = std::chrono::milliseconds(20000);

        /**
         * Triggers an event for the TargetController and waits for a response.
         *
         * To use this method, the triggered event must define a 'TargetControllerResponseType' alias, which should
         * specify the type of response expected from the TargetController.
         * For an example of this, see the Events::ExtractTargetDescriptor class.
         *
         * If the TargetController fails to respond within the given time specified by the timeout argument, or it
         * responds with an instance of Events::TargetControllerErrorOccurred, this function will throw an exception.
         *
         * @tparam TriggerEventType
         *
         * @param event
         *  Event to trigger.
         *
         * @param timeout
         *  The time, in milliseconds, to wait for the TargetController to respond to the event. If this is not
         *  supplied, this->defaultTimeout will be used.
         *
         * @return
         */
        template<class TriggerEventType>
        auto triggerTargetControllerEventAndWaitForResponse(
            const Events::SharedEventPointerNonConst<TriggerEventType> event,
            std::optional<std::chrono::milliseconds> timeout = {}
        ) {
            using Bloom::Events::SharedEventPointer;
            using Bloom::Events::TargetControllerErrorOccurred;

            using ResponseEventType = typename TriggerEventType::TargetControllerResponseType;

            bool deRegisterEventType = false;

            if (!this->eventListener.isEventTypeRegistered<ResponseEventType>()) {
                this->eventListener.registerEventType<ResponseEventType>();
                deRegisterEventType = true;
            }

            this->eventManager.triggerEvent(event);

            auto responseEvent = this->eventListener.waitForEvent<
                ResponseEventType,
                TargetControllerErrorOccurred
            >(timeout.value_or(this->defaultTimeout), event->id);

            if (deRegisterEventType) {
                this->eventListener.deRegisterEventType<ResponseEventType>();
            }

            if (!responseEvent.has_value()) {
                throw Bloom::Exceptions::Exception("Timed out waiting for response from TargetController.");
            }

            if (!std::holds_alternative<SharedEventPointer<ResponseEventType>>(responseEvent.value())) {
                if (std::holds_alternative<SharedEventPointer<TargetControllerErrorOccurred>>(responseEvent.value())) {
                    auto& tcErrorEvent = std::get<SharedEventPointer<TargetControllerErrorOccurred>>(responseEvent.value());
                    throw Bloom::Exceptions::Exception(tcErrorEvent->errorMessage);

                } else {
                    throw Bloom::Exceptions::Exception("Unexpected response from TargetController");
                }
            }

            return std::get<SharedEventPointer<ResponseEventType>>(responseEvent.value());
        }
    };
}
