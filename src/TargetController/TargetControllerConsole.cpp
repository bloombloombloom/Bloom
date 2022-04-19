#include "TargetControllerConsole.hpp"

#include "src/EventManager/Events/Events.hpp"

// Commands
#include "Commands/StopTargetExecution.hpp"
#include "Commands/ResumeTargetExecution.hpp"

#include "src/Logger/Logger.hpp"

namespace Bloom::TargetController
{
    using namespace Bloom::Targets;
    using namespace Bloom::Events;
    using namespace Bloom::Exceptions;

    using Commands::StopTargetExecution;
    using Commands::ResumeTargetExecution;

    TargetControllerConsole::TargetControllerConsole(EventListener& eventListener)
        : eventListener(eventListener)
    {}

    TargetControllerState TargetControllerConsole::getTargetControllerState() {
        return this->triggerTargetControllerEventAndWaitForResponse(
            std::make_shared<ReportTargetControllerState>()
        )->state;
    }

    bool TargetControllerConsole::isTargetControllerInService() noexcept {
        try {
            return this->getTargetControllerState() == TargetControllerState::ACTIVE;

        } catch (...) {
            return false;
        }
    }

    Targets::TargetDescriptor TargetControllerConsole::getTargetDescriptor() {
        return this->triggerTargetControllerEventAndWaitForResponse(
            std::make_shared<ExtractTargetDescriptor>()
        )->targetDescriptor;
    }

    void TargetControllerConsole::stopTargetExecution() {
        this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<StopTargetExecution>(),
            this->defaultTimeout
        );
    }

    void TargetControllerConsole::continueTargetExecution(std::optional<std::uint32_t> fromAddress) {
        auto resumeExecutionCommand = std::make_unique<ResumeTargetExecution>();

        if (fromAddress.has_value()) {
            resumeExecutionCommand->fromProgramCounter = fromAddress.value();
        }

        this->commandManager.sendCommandAndWaitForResponse(
            std::move(resumeExecutionCommand),
            this->defaultTimeout
        );
    }

    void TargetControllerConsole::stepTargetExecution(std::optional<std::uint32_t> fromAddress) {
        auto stepExecutionEvent = std::make_shared<StepTargetExecution>();

        if (fromAddress.has_value()) {
            stepExecutionEvent->fromProgramCounter = fromAddress.value();
        }

        this->triggerTargetControllerEventAndWaitForResponse(stepExecutionEvent);
    }

    TargetRegisters TargetControllerConsole::readRegisters(const TargetRegisterDescriptors& descriptors) {
        auto readRegistersEvent = std::make_shared<RetrieveRegistersFromTarget>();
        readRegistersEvent->descriptors = descriptors;

        return this->triggerTargetControllerEventAndWaitForResponse(readRegistersEvent)->registers;
    }

    void TargetControllerConsole::writeRegisters(const TargetRegisters& registers) {
        auto event = std::make_shared<WriteRegistersToTarget>();
        event->registers = std::move(registers);

        this->triggerTargetControllerEventAndWaitForResponse(event);
    }

    TargetMemoryBuffer TargetControllerConsole::readMemory(
        TargetMemoryType memoryType,
        std::uint32_t startAddress,
        std::uint32_t bytes,
        const std::set<Targets::TargetMemoryAddressRange>& excludedAddressRanges
    ) {
        auto readMemoryEvent = std::make_shared<RetrieveMemoryFromTarget>();
        readMemoryEvent->memoryType = memoryType;
        readMemoryEvent->startAddress = startAddress;
        readMemoryEvent->bytes = bytes;
        readMemoryEvent->excludedAddressRanges = excludedAddressRanges;

        return this->triggerTargetControllerEventAndWaitForResponse(readMemoryEvent)->data;
    }

    void TargetControllerConsole::writeMemory(
        TargetMemoryType memoryType,
        std::uint32_t startAddress,
        const TargetMemoryBuffer& buffer
    ) {
        auto writeMemoryEvent = std::make_shared<WriteMemoryToTarget>();
        writeMemoryEvent->memoryType = memoryType;
        writeMemoryEvent->startAddress = startAddress;
        writeMemoryEvent->buffer = buffer;

        this->triggerTargetControllerEventAndWaitForResponse(writeMemoryEvent);
    }

    void TargetControllerConsole::setBreakpoint(TargetBreakpoint breakpoint) {
        auto event = std::make_shared<SetBreakpointOnTarget>();
        event->breakpoint = breakpoint;

        this->triggerTargetControllerEventAndWaitForResponse(event);
    }

    void TargetControllerConsole::removeBreakpoint(TargetBreakpoint breakpoint) {
        auto event = std::make_shared<RemoveBreakpointOnTarget>();
        event->breakpoint = breakpoint;

        this->triggerTargetControllerEventAndWaitForResponse(event);
    }

    void TargetControllerConsole::requestPinStates(int variantId) {
        auto requestEvent = std::make_shared<RetrieveTargetPinStates>();
        requestEvent->variantId = variantId;

        EventManager::triggerEvent(requestEvent);
    }

    Targets::TargetPinStateMappingType TargetControllerConsole::getPinStates(int variantId) {
        auto requestEvent = std::make_shared<RetrieveTargetPinStates>();
        requestEvent->variantId = variantId;

        return this->triggerTargetControllerEventAndWaitForResponse(requestEvent)->pinSatesByNumber;
    }

    void TargetControllerConsole::setPinState(TargetPinDescriptor pinDescriptor, TargetPinState pinState) {
        auto updateEvent = std::make_shared<SetTargetPinState>();
        updateEvent->pinDescriptor = std::move(pinDescriptor);
        updateEvent->pinState = pinState;

        this->triggerTargetControllerEventAndWaitForResponse(updateEvent);
    }

    std::uint32_t TargetControllerConsole::getStackPointer() {
        return this->triggerTargetControllerEventAndWaitForResponse(
            std::make_shared<RetrieveStackPointerFromTarget>()
        )->stackPointer;
    }

    void TargetControllerConsole::resetTarget() {
        this->triggerTargetControllerEventAndWaitForResponse(std::make_shared<ResetTarget>());
    }
}
