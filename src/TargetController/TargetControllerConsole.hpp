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
     * The TargetControllerConsole provides an interface to the TargetController, for components within Bloom that
     * require access to common functionality from the TargetController.
     */
    class TargetControllerConsole
    {
    private:
        EventManager& eventManager;
        EventListener& eventListener;

        std::chrono::milliseconds defaultTimeout = std::chrono::milliseconds(10000);

        /**
         * Triggers an event for the TargetController and waits for a response.
         *
         * To use this method, the triggered event must define a 'TargetControllerResponseType' alias, which should
         * specify the type of response expected by the TargetController.
         * For an example of this, see the Events::ExtractTargetDescriptor class.
         *
         * If the TargetController fails to respond within the given time specified by the timeout parameter, or it
         * responds with an instance of Events::TargetControllerErrorOccurred, this method will throw an exception.
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
            using ResponseEventType = typename TriggerEventType::TargetControllerResponseType;

            bool deRegisterEventType = false;

            if (!this->eventListener.isEventTypeRegistered<ResponseEventType>()) {
                this->eventListener.registerEventType<ResponseEventType>();
                deRegisterEventType = true;
            }

            this->eventManager.triggerEvent(event);

            auto responseEvent = this->eventListener.waitForEvent<
                ResponseEventType,
                Events::TargetControllerErrorOccurred
            >(timeout.value_or(this->defaultTimeout), event->id);

            if (deRegisterEventType) {
                this->eventListener.deRegisterEventType<ResponseEventType>();
            }

            if (!responseEvent.has_value()) {
                throw Bloom::Exceptions::Exception("Timed out waiting for response from TargetController.");
            }

            if (!std::holds_alternative<Events::SharedEventPointer<ResponseEventType>>(responseEvent.value())) {
                throw Bloom::Exceptions::Exception("Unexpected response from TargetController");
            }

            return std::get<Events::SharedEventPointer<ResponseEventType>>(responseEvent.value());
        }

    public:
        TargetControllerConsole(EventManager& eventManager, EventListener& eventListener):
        eventManager(eventManager), eventListener(eventListener) {};

        void setDefaultTimeout(std::chrono::milliseconds timeout) {
            this->defaultTimeout = timeout;
        }

        TargetControllerState getTargetControllerState();

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
         * Requests a pin state update on the target, for a specific pin.
         *
         * @param variantId
         * @param pinDescriptor
         * @param pinState
         */
        void setPinState(int variantId, Targets::TargetPinDescriptor pinDescriptor, Targets::TargetPinState pinState);

        /**
         * Requests a pin state refresh from the TargetController, for a specific target variant.
         *
         * @param variantId
         */
        void requestPinStates(int variantId);
    };
}
