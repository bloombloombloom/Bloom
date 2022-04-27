#include "TargetControllerConsole.hpp"

#include "src/EventManager/Events/Events.hpp"

#include "TargetControllerComponent.hpp"

// Commands
#include "Commands/StopTargetExecution.hpp"
#include "Commands/ResumeTargetExecution.hpp"
#include "Commands/ResetTarget.hpp"
#include "Commands/ReadTargetRegisters.hpp"
#include "Commands/WriteTargetRegisters.hpp"
#include "Commands/ReadTargetMemory.hpp"

#include "src/Logger/Logger.hpp"

namespace Bloom::TargetController
{
    using namespace Bloom::Targets;
    using namespace Bloom::Events;
    using namespace Bloom::Exceptions;

    using Commands::StopTargetExecution;
    using Commands::ResumeTargetExecution;
    using Commands::ResetTarget;
    using Commands::ReadTargetRegisters;
    using Commands::WriteTargetRegisters;
    using Commands::ReadTargetMemory;

    TargetControllerConsole::TargetControllerConsole(EventListener& eventListener)
        : eventListener(eventListener)
    {}

    TargetControllerState TargetControllerConsole::getTargetControllerState() {
        return TargetControllerComponent::getState();
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
        return this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<ReadTargetRegisters>(descriptors),
            this->defaultTimeout
        )->registers;
    }

    void TargetControllerConsole::writeRegisters(const TargetRegisters& registers) {
        this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<WriteTargetRegisters>(registers),
            this->defaultTimeout
        );
    }

    TargetMemoryBuffer TargetControllerConsole::readMemory(
        TargetMemoryType memoryType,
        std::uint32_t startAddress,
        std::uint32_t bytes,
        const std::set<Targets::TargetMemoryAddressRange>& excludedAddressRanges
    ) {
        return this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<ReadTargetMemory>(
                memoryType,
                startAddress,
                bytes,
                excludedAddressRanges
            ),
            this->defaultTimeout
        )->data;
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
        this->commandManager.sendCommandAndWaitForResponse(
            std::make_unique<ResetTarget>(),
            this->defaultTimeout
        );
    }
}
