#include <variant>
#include <cstdint>

#include "DebugServer.hpp"
#include "src/Exceptions/InvalidConfig.hpp"
#include "src/Logger/Logger.hpp"

using namespace Bloom::DebugServers;

void DebugServer::run() {
    try {
        this->startup();

        Logger::info("DebugServer ready");

        while (this->getState() == ThreadState::READY) {
            this->serve();
            this->eventListener->dispatchCurrentEvents();
        }
    } catch (const std::exception& exception) {
        Logger::error("DebugServer fatal error: " + std::string(exception.what()));
    }

    this->shutdown();
}

void DebugServer::startup() {
    this->setName("DS");
    Logger::info("Starting DebugServer");

    this->eventManager.registerListener(this->eventListener);

    this->interruptEventNotifier = std::make_shared<EventNotifier>();
    this->interruptEventNotifier->init();
    this->eventListener->setInterruptEventNotifier(this->interruptEventNotifier);

    // Register event handlers
    this->eventListener->registerCallbackForEventType<Events::ShutdownDebugServer>(
        std::bind(&DebugServer::onShutdownDebugServerEvent, this, std::placeholders::_1)
    );

    this->init();
    this->setStateAndEmitEvent(ThreadState::READY);
}

void DebugServer::shutdown() {
    if (this->getState() == ThreadState::STOPPED || this->getState() == ThreadState::SHUTDOWN_INITIATED) {
        return;
    }

    this->setState(ThreadState::SHUTDOWN_INITIATED);
    Logger::info("Shutting down DebugServer");
    this->close();
    this->interruptEventNotifier->close();
    this->setStateAndEmitEvent(ThreadState::STOPPED);
}

void DebugServer::onShutdownDebugServerEvent(EventPointer<Events::ShutdownDebugServer> event) {
    this->shutdown();
}

void DebugServer::stopTargetExecution() {
    auto stopTargetEvent = std::make_shared<Events::StopTargetExecution>();
    this->eventManager.triggerEvent(stopTargetEvent);

    auto responseEvent = this->eventListener->waitForEvent<
        Events::TargetExecutionStopped,
        Events::TargetControllerErrorOccurred
    >(std::chrono::milliseconds(5000), stopTargetEvent->id);

    if (!responseEvent.has_value()
        || !std::holds_alternative<EventPointer<Events::TargetExecutionStopped>>(responseEvent.value())
    ) {
        throw Exception("Unexpected response from TargetController");
    }
}

void DebugServer::continueTargetExecution(std::optional<std::uint32_t> fromAddress) {
    auto resumeExecutionEvent = std::make_shared<Events::ResumeTargetExecution>();

    if (fromAddress.has_value()) {
        resumeExecutionEvent->fromProgramCounter = fromAddress.value();
    }

    this->eventManager.triggerEvent(resumeExecutionEvent);
    auto responseEvent = this->eventListener->waitForEvent<
        Events::TargetExecutionResumed,
        Events::TargetControllerErrorOccurred
    >(std::chrono::milliseconds(5000), resumeExecutionEvent->id);

    if (!responseEvent.has_value()
        || !std::holds_alternative<EventPointer<Events::TargetExecutionResumed>>(responseEvent.value())
    ) {
        throw Exception("Unexpected response from TargetController");
    }
}

void DebugServer::stepTargetExecution(std::optional<std::uint32_t> fromAddress) {
    auto stepExecutionEvent = std::make_shared<Events::StepTargetExecution>();

    if (fromAddress.has_value()) {
        stepExecutionEvent->fromProgramCounter = fromAddress.value();
    }

    this->eventManager.triggerEvent(stepExecutionEvent);
    auto responseEvent = this->eventListener->waitForEvent<
        Events::TargetExecutionResumed,
        Events::TargetControllerErrorOccurred
    >(std::chrono::milliseconds(5000), stepExecutionEvent->id);

    if (!responseEvent.has_value()
        || !std::holds_alternative<EventPointer<Events::TargetExecutionResumed>>(responseEvent.value())
    ) {
        throw Exception("Unexpected response from TargetController");
    }
}

TargetRegisters DebugServer::readGeneralRegistersFromTarget(TargetRegisterDescriptors descriptors) {
    auto readRegistersEvent = std::make_shared<Events::RetrieveRegistersFromTarget>();
    readRegistersEvent->descriptors = descriptors;

    this->eventManager.triggerEvent(readRegistersEvent);
    auto responseEvent = this->eventListener->waitForEvent<
        Events::RegistersRetrievedFromTarget,
        Events::TargetControllerErrorOccurred
    >(std::chrono::milliseconds(5000), readRegistersEvent->id);

    if (!responseEvent.has_value()
        || !std::holds_alternative<EventPointer<Events::RegistersRetrievedFromTarget>>(responseEvent.value())
    ) {
        throw Exception("Unexpected response from TargetController");
    }

    auto retrievedRegistersEvent = std::get<EventPointer<Events::RegistersRetrievedFromTarget>>(responseEvent.value());
    return retrievedRegistersEvent->registers;
}

void DebugServer::writeGeneralRegistersToTarget(TargetRegisters registers) {
    auto event = std::make_shared<Events::WriteRegistersToTarget>();
    event->registers = registers;

    this->eventManager.triggerEvent(event);
    auto responseEvent = this->eventListener->waitForEvent<
        Events::RegistersWrittenToTarget,
        Events::TargetControllerErrorOccurred
    >(std::chrono::milliseconds(5000), event->id);

    if (!responseEvent.has_value()
        || !std::holds_alternative<EventPointer<Events::RegistersWrittenToTarget>>(responseEvent.value())
    ) {
        throw Exception("Unexpected response from TargetController");
    }
}

TargetMemoryBuffer DebugServer::readMemoryFromTarget(TargetMemoryType memoryType, std::uint32_t startAddress, std::uint32_t bytes) {
    auto readMemoryEvent = std::make_shared<Events::RetrieveMemoryFromTarget>();
    readMemoryEvent->memoryType = memoryType;
    readMemoryEvent->startAddress = startAddress;
    readMemoryEvent->bytes = bytes;

    this->eventManager.triggerEvent(readMemoryEvent);
    auto responseEvent = this->eventListener->waitForEvent<
        Events::MemoryRetrievedFromTarget,
        Events::TargetControllerErrorOccurred
    >(std::chrono::milliseconds(5000), readMemoryEvent->id);

    if (!responseEvent.has_value()
        || !std::holds_alternative<EventPointer<Events::MemoryRetrievedFromTarget>>(responseEvent.value())
    ) {
        throw Exception("Unexpected response from TargetController");
    }

    auto retrievedRegistersEvent = std::get<EventPointer<Events::MemoryRetrievedFromTarget>>(responseEvent.value());
    return retrievedRegistersEvent->data;
}

void DebugServer::writeMemoryToTarget(
    TargetMemoryType memoryType,
    std::uint32_t startAddress,
    const TargetMemoryBuffer& buffer
) {
    auto writeMemoryEvent = std::make_shared<Events::WriteMemoryToTarget>();
    writeMemoryEvent->memoryType = memoryType;
    writeMemoryEvent->startAddress = startAddress;
    writeMemoryEvent->buffer = buffer;

    this->eventManager.triggerEvent(writeMemoryEvent);
    auto responseEvent = this->eventListener->waitForEvent<
        Events::MemoryWrittenToTarget,
        Events::TargetControllerErrorOccurred
    >(std::chrono::milliseconds(5000), writeMemoryEvent->id);

    if (!responseEvent.has_value()
        || !std::holds_alternative<EventPointer<Events::MemoryWrittenToTarget>>(responseEvent.value())
    ) {
        throw Exception("Unexpected response from TargetController");
    }
}

void DebugServer::setBreakpointOnTarget(TargetBreakpoint breakpoint) {
    auto event = std::make_shared<Events::SetBreakpointOnTarget>();
    event->breakpoint = breakpoint;

    this->eventManager.triggerEvent(event);
    auto responseEvent = this->eventListener->waitForEvent<
        Events::BreakpointSetOnTarget,
        Events::TargetControllerErrorOccurred
    >(std::chrono::milliseconds(5000), event->id);

    if (!responseEvent.has_value()
        || !std::holds_alternative<EventPointer<Events::BreakpointSetOnTarget>>(responseEvent.value())
    ) {
        throw Exception("Unexpected response from TargetController");
    }
}

void DebugServer::removeBreakpointOnTarget(TargetBreakpoint breakpoint) {
    auto event = std::make_shared<Events::RemoveBreakpointOnTarget>();
    event->breakpoint = breakpoint;

    this->eventManager.triggerEvent(event);
    auto responseEvent = this->eventListener->waitForEvent<
        Events::BreakpointRemovedOnTarget,
        Events::TargetControllerErrorOccurred
    >(std::chrono::milliseconds(5000), event->id);

    if (!responseEvent.has_value()
        || !std::holds_alternative<EventPointer<Events::BreakpointRemovedOnTarget>>(responseEvent.value())
    ) {
        throw Exception("Unexpected response from TargetController");
    }
}

