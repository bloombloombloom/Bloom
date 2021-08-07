#include <cstdint>

#include "TargetControllerConsole.hpp"
#include "src/EventManager/Events/Events.hpp"
#include "src/Logger/Logger.hpp"

using namespace Bloom;
using namespace Bloom::Targets;
using namespace Bloom::Events;
using namespace Bloom::Exceptions;

TargetControllerState TargetControllerConsole::getTargetControllerState() {
    auto getStateEvent = std::make_shared<ReportTargetControllerState>();
    this->eventManager.triggerEvent(getStateEvent);
    auto responseEvent = this->eventListener.waitForEvent<
        TargetControllerStateReported,
        TargetControllerErrorOccurred
    >(this->defaultTimeout, getStateEvent->id);

    if (!responseEvent.has_value()
        || !std::holds_alternative<SharedEventPointer<TargetControllerStateReported>>(responseEvent.value())
    ) {
        throw Exception("Unexpected response from TargetController");
    }

    auto stateReportedEvent = std::get<SharedEventPointer<TargetControllerStateReported>>(responseEvent.value());
    return stateReportedEvent->state;
}

bool TargetControllerConsole::isTargetControllerInService() noexcept {
    try {
        return this->getTargetControllerState() == TargetControllerState::ACTIVE;

    } catch (const std::runtime_error&) {
        return false;
    }
}

Targets::TargetDescriptor TargetControllerConsole::getTargetDescriptor() {
    auto extractEvent = std::make_shared<ExtractTargetDescriptor>();
    this->eventManager.triggerEvent(extractEvent);
    auto responseEvent = this->eventListener.waitForEvent<
        TargetDescriptorExtracted,
        TargetControllerErrorOccurred
    >(this->defaultTimeout, extractEvent->id);

    if (!responseEvent.has_value()
        || !std::holds_alternative<SharedEventPointer<TargetDescriptorExtracted>>(responseEvent.value())
    ) {
        throw Exception("Unexpected response from TargetController");
    }

    auto descriptorExtracted = std::get<SharedEventPointer<TargetDescriptorExtracted>>(responseEvent.value());
    return descriptorExtracted->targetDescriptor;
}

void TargetControllerConsole::stopTargetExecution() {
    auto stopTargetEvent = std::make_shared<StopTargetExecution>();
    this->eventManager.triggerEvent(stopTargetEvent);

    auto responseEvent = this->eventListener.waitForEvent<
        TargetExecutionStopped,
        TargetControllerErrorOccurred
    >(this->defaultTimeout, stopTargetEvent->id);

    if (!responseEvent.has_value()
        || !std::holds_alternative<SharedEventPointer<TargetExecutionStopped>>(responseEvent.value())
    ) {
        throw Exception("Unexpected response from TargetController");
    }
}

void TargetControllerConsole::continueTargetExecution(std::optional<std::uint32_t> fromAddress) {
    auto resumeExecutionEvent = std::make_shared<ResumeTargetExecution>();

    if (fromAddress.has_value()) {
        resumeExecutionEvent->fromProgramCounter = fromAddress.value();
    }

    this->eventManager.triggerEvent(resumeExecutionEvent);
    auto responseEvent = this->eventListener.waitForEvent<
        TargetExecutionResumed,
        TargetControllerErrorOccurred
    >(this->defaultTimeout, resumeExecutionEvent->id);

    if (!responseEvent.has_value()
        || !std::holds_alternative<SharedEventPointer<TargetExecutionResumed>>(responseEvent.value())
    ) {
        throw Exception("Unexpected response from TargetController");
    }
}

void TargetControllerConsole::stepTargetExecution(std::optional<std::uint32_t> fromAddress) {
    auto stepExecutionEvent = std::make_shared<StepTargetExecution>();

    if (fromAddress.has_value()) {
        stepExecutionEvent->fromProgramCounter = fromAddress.value();
    }

    this->eventManager.triggerEvent(stepExecutionEvent);
    auto responseEvent = this->eventListener.waitForEvent<
        TargetExecutionResumed,
        TargetControllerErrorOccurred
    >(this->defaultTimeout, stepExecutionEvent->id);

    if (!responseEvent.has_value()
        || !std::holds_alternative<SharedEventPointer<TargetExecutionResumed>>(responseEvent.value())
    ) {
        throw Exception("Unexpected response from TargetController");
    }
}

TargetRegisters TargetControllerConsole::readRegisters(const TargetRegisterDescriptors& descriptors) {
    auto readRegistersEvent = std::make_shared<RetrieveRegistersFromTarget>();
    readRegistersEvent->descriptors = descriptors;

    this->eventManager.triggerEvent(readRegistersEvent);
    auto responseEvent = this->eventListener.waitForEvent<
        RegistersRetrievedFromTarget,
        TargetControllerErrorOccurred
    >(this->defaultTimeout, readRegistersEvent->id);

    if (!responseEvent.has_value()
        || !std::holds_alternative<SharedEventPointer<RegistersRetrievedFromTarget>>(responseEvent.value())
    ) {
        throw Exception("Unexpected response from TargetController");
    }

    auto retrievedRegistersEvent = std::get<SharedEventPointer<RegistersRetrievedFromTarget>>(
        responseEvent.value()
    );
    return retrievedRegistersEvent->registers;
}

void TargetControllerConsole::writeRegisters(const TargetRegisters& registers) {
    auto event = std::make_shared<WriteRegistersToTarget>();
    event->registers = std::move(registers);

    this->eventManager.triggerEvent(event);
    auto responseEvent = this->eventListener.waitForEvent<
        RegistersWrittenToTarget,
        TargetControllerErrorOccurred
    >(this->defaultTimeout, event->id);

    if (!responseEvent.has_value()
        || !std::holds_alternative<SharedEventPointer<RegistersWrittenToTarget>>(responseEvent.value())
    ) {
        throw Exception("Unexpected response from TargetController");
    }
}

TargetMemoryBuffer TargetControllerConsole::readMemory(
    TargetMemoryType memoryType,
    std::uint32_t startAddress,
    std::uint32_t bytes
) {
    auto readMemoryEvent = std::make_shared<RetrieveMemoryFromTarget>();
    readMemoryEvent->memoryType = memoryType;
    readMemoryEvent->startAddress = startAddress;
    readMemoryEvent->bytes = bytes;

    this->eventManager.triggerEvent(readMemoryEvent);
    auto responseEvent = this->eventListener.waitForEvent<
        MemoryRetrievedFromTarget,
        TargetControllerErrorOccurred
    >(this->defaultTimeout, readMemoryEvent->id);

    if (!responseEvent.has_value()
        || !std::holds_alternative<SharedEventPointer<MemoryRetrievedFromTarget>>(responseEvent.value())
    ) {
        throw Exception("Unexpected response from TargetController");
    }

    auto retrievedRegistersEvent = std::get<SharedEventPointer<MemoryRetrievedFromTarget>>(
        responseEvent.value()
    );
    return retrievedRegistersEvent->data;
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

    this->eventManager.triggerEvent(writeMemoryEvent);
    auto responseEvent = this->eventListener.waitForEvent<
        MemoryWrittenToTarget,
        TargetControllerErrorOccurred
    >(this->defaultTimeout, writeMemoryEvent->id);

    if (!responseEvent.has_value()
        || !std::holds_alternative<SharedEventPointer<MemoryWrittenToTarget>>(responseEvent.value())
    ) {
        throw Exception("Unexpected response from TargetController");
    }
}

void TargetControllerConsole::setBreakpoint(TargetBreakpoint breakpoint) {
    auto event = std::make_shared<SetBreakpointOnTarget>();
    event->breakpoint = breakpoint;

    this->eventManager.triggerEvent(event);
    auto responseEvent = this->eventListener.waitForEvent<
        BreakpointSetOnTarget,
        TargetControllerErrorOccurred
    >(this->defaultTimeout, event->id);

    if (!responseEvent.has_value()
        || !std::holds_alternative<SharedEventPointer<BreakpointSetOnTarget>>(responseEvent.value())
    ) {
        throw Exception("Unexpected response from TargetController");
    }
}

void TargetControllerConsole::removeBreakpoint(TargetBreakpoint breakpoint) {
    auto event = std::make_shared<RemoveBreakpointOnTarget>();
    event->breakpoint = breakpoint;

    this->eventManager.triggerEvent(event);
    auto responseEvent = this->eventListener.waitForEvent<
        BreakpointRemovedOnTarget,
        TargetControllerErrorOccurred
    >(this->defaultTimeout, event->id);

    if (!responseEvent.has_value()
        || !std::holds_alternative<SharedEventPointer<BreakpointRemovedOnTarget>>(responseEvent.value())
    ) {
        throw Exception("Unexpected response from TargetController");
    }
}

void TargetControllerConsole::requestPinStates(int variantId) {
    auto requestEvent = std::make_shared<RetrieveTargetPinStates>();
    requestEvent->variantId = variantId;

    this->eventManager.triggerEvent(requestEvent);
}

void TargetControllerConsole::setPinState(int variantId, TargetPinDescriptor pinDescriptor, TargetPinState pinState) {
    auto updateEvent = std::make_shared<SetTargetPinState>();
    updateEvent->variantId = variantId;
    updateEvent->pinDescriptor = std::move(pinDescriptor);
    updateEvent->pinState = pinState;

    this->eventManager.triggerEvent(updateEvent);
}
