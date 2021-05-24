#include <cstdint>

#include "TargetControllerConsole.hpp"
#include "src/EventManager/Events/Events.hpp"
#include "src/Logger/Logger.hpp"

using namespace Bloom;
using namespace Bloom::Targets;
using namespace Bloom::Events;
using namespace Bloom::Exceptions;

Targets::TargetDescriptor TargetControllerConsole::getTargetDescriptor() {
    auto extractEvent = std::make_shared<Events::ExtractTargetDescriptor>();
    this->eventManager.triggerEvent(extractEvent);
    auto responseEvent = this->eventListener.waitForEvent<
        Events::TargetDescriptorExtracted,
        Events::TargetControllerErrorOccurred
    >(this->defaultTimeout, extractEvent->id);

    if (!responseEvent.has_value()
        || !std::holds_alternative<EventPointer<Events::TargetDescriptorExtracted>>(responseEvent.value())
    ) {
        throw Exception("Unexpected response from TargetController");
    }

    auto descriptorExtracted = std::get<EventPointer<Events::TargetDescriptorExtracted>>(responseEvent.value());
    return descriptorExtracted->targetDescriptor;
}

void TargetControllerConsole::stopTargetExecution() {
    auto stopTargetEvent = std::make_shared<Events::StopTargetExecution>();
    this->eventManager.triggerEvent(stopTargetEvent);

    auto responseEvent = this->eventListener.waitForEvent<
        Events::TargetExecutionStopped,
        Events::TargetControllerErrorOccurred
    >(this->defaultTimeout, stopTargetEvent->id);

    if (!responseEvent.has_value()
        || !std::holds_alternative<EventPointer<Events::TargetExecutionStopped>>(responseEvent.value())
    ) {
        throw Exception("Unexpected response from TargetController");
    }
}

void TargetControllerConsole::continueTargetExecution(std::optional<std::uint32_t> fromAddress) {
    auto resumeExecutionEvent = std::make_shared<Events::ResumeTargetExecution>();

    if (fromAddress.has_value()) {
        resumeExecutionEvent->fromProgramCounter = fromAddress.value();
    }

    this->eventManager.triggerEvent(resumeExecutionEvent);
    auto responseEvent = this->eventListener.waitForEvent<
        Events::TargetExecutionResumed,
        Events::TargetControllerErrorOccurred
    >(this->defaultTimeout, resumeExecutionEvent->id);

    if (!responseEvent.has_value()
        || !std::holds_alternative<EventPointer<Events::TargetExecutionResumed>>(responseEvent.value())
    ) {
        throw Exception("Unexpected response from TargetController");
    }
}

void TargetControllerConsole::stepTargetExecution(std::optional<std::uint32_t> fromAddress) {
    auto stepExecutionEvent = std::make_shared<Events::StepTargetExecution>();

    if (fromAddress.has_value()) {
        stepExecutionEvent->fromProgramCounter = fromAddress.value();
    }

    this->eventManager.triggerEvent(stepExecutionEvent);
    auto responseEvent = this->eventListener.waitForEvent<
        Events::TargetExecutionResumed,
        Events::TargetControllerErrorOccurred
    >(this->defaultTimeout, stepExecutionEvent->id);

    if (!responseEvent.has_value()
        || !std::holds_alternative<EventPointer<Events::TargetExecutionResumed>>(responseEvent.value())
    ) {
        throw Exception("Unexpected response from TargetController");
    }
}

TargetRegisters TargetControllerConsole::readGeneralRegisters(TargetRegisterDescriptors descriptors) {
    auto readRegistersEvent = std::make_shared<Events::RetrieveRegistersFromTarget>();
    readRegistersEvent->descriptors = descriptors;

    this->eventManager.triggerEvent(readRegistersEvent);
    auto responseEvent = this->eventListener.waitForEvent<
        Events::RegistersRetrievedFromTarget,
        Events::TargetControllerErrorOccurred
    >(this->defaultTimeout, readRegistersEvent->id);

    if (!responseEvent.has_value()
        || !std::holds_alternative<EventPointer<Events::RegistersRetrievedFromTarget>>(responseEvent.value())
    ) {
        throw Exception("Unexpected response from TargetController");
    }

    auto retrievedRegistersEvent = std::get<EventPointer<Events::RegistersRetrievedFromTarget>>(responseEvent.value());
    return retrievedRegistersEvent->registers;
}

void TargetControllerConsole::writeGeneralRegisters(TargetRegisters registers) {
    auto event = std::make_shared<Events::WriteRegistersToTarget>();
    event->registers = registers;

    this->eventManager.triggerEvent(event);
    auto responseEvent = this->eventListener.waitForEvent<
        Events::RegistersWrittenToTarget,
        Events::TargetControllerErrorOccurred
    >(this->defaultTimeout, event->id);

    if (!responseEvent.has_value()
        || !std::holds_alternative<EventPointer<Events::RegistersWrittenToTarget>>(responseEvent.value())
    ) {
        throw Exception("Unexpected response from TargetController");
    }
}

TargetMemoryBuffer TargetControllerConsole::readMemory(
    TargetMemoryType memoryType,
    std::uint32_t startAddress,
    std::uint32_t bytes
) {
    auto readMemoryEvent = std::make_shared<Events::RetrieveMemoryFromTarget>();
    readMemoryEvent->memoryType = memoryType;
    readMemoryEvent->startAddress = startAddress;
    readMemoryEvent->bytes = bytes;

    this->eventManager.triggerEvent(readMemoryEvent);
    auto responseEvent = this->eventListener.waitForEvent<
        Events::MemoryRetrievedFromTarget,
        Events::TargetControllerErrorOccurred
    >(this->defaultTimeout, readMemoryEvent->id);

    if (!responseEvent.has_value()
        || !std::holds_alternative<EventPointer<Events::MemoryRetrievedFromTarget>>(responseEvent.value())
    ) {
        throw Exception("Unexpected response from TargetController");
    }

    auto retrievedRegistersEvent = std::get<EventPointer<Events::MemoryRetrievedFromTarget>>(responseEvent.value());
    return retrievedRegistersEvent->data;
}

void TargetControllerConsole::writeMemory(
    TargetMemoryType memoryType,
    std::uint32_t startAddress,
    const TargetMemoryBuffer& buffer
) {
    auto writeMemoryEvent = std::make_shared<Events::WriteMemoryToTarget>();
    writeMemoryEvent->memoryType = memoryType;
    writeMemoryEvent->startAddress = startAddress;
    writeMemoryEvent->buffer = buffer;

    this->eventManager.triggerEvent(writeMemoryEvent);
    auto responseEvent = this->eventListener.waitForEvent<
        Events::MemoryWrittenToTarget,
        Events::TargetControllerErrorOccurred
    >(this->defaultTimeout, writeMemoryEvent->id);

    if (!responseEvent.has_value()
        || !std::holds_alternative<EventPointer<Events::MemoryWrittenToTarget>>(responseEvent.value())
    ) {
        throw Exception("Unexpected response from TargetController");
    }
}

void TargetControllerConsole::setBreakpoint(TargetBreakpoint breakpoint) {
    auto event = std::make_shared<Events::SetBreakpointOnTarget>();
    event->breakpoint = breakpoint;

    this->eventManager.triggerEvent(event);
    auto responseEvent = this->eventListener.waitForEvent<
        Events::BreakpointSetOnTarget,
        Events::TargetControllerErrorOccurred
    >(this->defaultTimeout, event->id);

    if (!responseEvent.has_value()
        || !std::holds_alternative<EventPointer<Events::BreakpointSetOnTarget>>(responseEvent.value())
    ) {
        throw Exception("Unexpected response from TargetController");
    }
}

void TargetControllerConsole::removeBreakpoint(TargetBreakpoint breakpoint) {
    auto event = std::make_shared<Events::RemoveBreakpointOnTarget>();
    event->breakpoint = breakpoint;

    this->eventManager.triggerEvent(event);
    auto responseEvent = this->eventListener.waitForEvent<
        Events::BreakpointRemovedOnTarget,
        Events::TargetControllerErrorOccurred
    >(this->defaultTimeout, event->id);

    if (!responseEvent.has_value()
        || !std::holds_alternative<EventPointer<Events::BreakpointRemovedOnTarget>>(responseEvent.value())
    ) {
        throw Exception("Unexpected response from TargetController");
    }
}

void TargetControllerConsole::requestPinStates(int variantId) {
    auto requestEvent = std::make_shared<Events::RetrieveTargetPinStates>();
    requestEvent->variantId = variantId;

    this->eventManager.triggerEvent(requestEvent);
}

void TargetControllerConsole::setPinState(int variantId, TargetPinDescriptor pinDescriptor, TargetPinState pinState) {
    auto updateEvent = std::make_shared<Events::SetTargetPinState>();
    updateEvent->variantId = variantId;
    updateEvent->pinDescriptor = pinDescriptor;
    updateEvent->pinState = pinState;

    this->eventManager.triggerEvent(updateEvent);
}