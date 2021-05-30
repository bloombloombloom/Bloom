#include <thread>
#include <filesystem>
#include <typeindex>

#include "TargetController.hpp"
#include "src/Exceptions/InvalidConfig.hpp"
#include "src/Exceptions/TargetControllerStartupFailure.hpp"
#include "src/Exceptions/DeviceCommunicationFailure.hpp"
#include "src/Application.hpp"
#include "src/Helpers/Paths.hpp"

using namespace Bloom;
using namespace Bloom::Targets;
using namespace Bloom::Events;
using namespace Bloom::Exceptions;

void TargetController::run() {
    try {
        this->startup();

        this->setThreadStateAndEmitEvent(ThreadState::READY);
        Logger::debug("TargetController ready and waiting for events.");

        while (this->getThreadState() == ThreadState::READY) {
            try {
                if (this->state == TargetControllerState::ACTIVE) {
                    this->fireTargetEvents();
                }

                this->eventListener->waitAndDispatch(60);

            } catch (const DeviceCommunicationFailure& exception) {
                /*
                 * Upon a device communication failure, we assume Bloom has lost control of the debug tool. This could
                 * be the result of the user disconnecting the debug tool, or issuing a soft reset.
                 * The soft reset could have been issued via another application, without the user's knowledge.
                 * See https://github.com/navnavnav/Bloom/issues/3 for more on that.
                 *
                 * The TC will go into a suspended state and the DebugServer should terminate any active debug
                 * session. When the user attempts to start another debug session, we will try to re-connect to the
                 * debug tool.
                 */
                this->suspend();
            }
        }

    } catch (const TargetControllerStartupFailure& exception) {
        Logger::error("TargetController failed to startup. See below for errors:");
        Logger::error(exception.getMessage());

    } catch (const Exception& exception) {
        Logger::error("The TargetController encountered a fatal error. See below for errors:");
        Logger::error(exception.getMessage());

    } catch (const std::exception& exception) {
        Logger::error("The TargetController encountered a fatal error. See below for errors:");
        Logger::error(std::string(exception.what()));
    }

    this->shutdown();
}

void TargetController::startup() {
    this->setName("TC");
    Logger::info("Starting TargetController");
    this->setThreadState(ThreadState::STARTING);
    this->blockAllSignalsOnCurrentThread();
    this->eventManager.registerListener(this->eventListener);

    // Install Bloom's udev rules if not already installed
    this->checkUdevRules();

    // Register event handlers
    this->eventListener->registerCallbackForEventType<Events::ReportTargetControllerState>(
        std::bind(&TargetController::onStateReportRequest, this, std::placeholders::_1)
    );

    this->eventListener->registerCallbackForEventType<Events::ShutdownTargetController>(
        std::bind(&TargetController::onShutdownTargetControllerEvent, this, std::placeholders::_1)
    );

    this->eventListener->registerCallbackForEventType<Events::DebugSessionStarted>(
        std::bind(&TargetController::onDebugSessionStartedEvent, this, std::placeholders::_1)
    );

    this->resume();
}

void TargetController::checkUdevRules() {
    auto bloomRulesPath = std::string("/etc/udev/rules.d/99-bloom.rules");
    auto latestBloomRulesPath = Paths::resourcesDirPath() + "/UDevRules/99-bloom.rules";

    if (!std::filesystem::exists(bloomRulesPath)) {
        Logger::warning("Bloom udev rules missing - attempting installation");

        // We can only install them if we're running as root
        if (!Application::isRunningAsRoot()) {
            Logger::error("Bloom udev rules missing - cannot install udev rules without root privileges.\n"
                "Running Bloom once with root privileges will allow it to automatically install the udev rules."
                " Alternatively, instructions on manually installing the udev rules can be found "
                "here: https://bloom.oscillate.io/docs/getting-started\nBloom may fail to connect to some debug tools "
                "until this is resolved.");
            return;
        }

        if (!std::filesystem::exists(latestBloomRulesPath)) {
            // This shouldn't happen, but it can if someone has been messing with the installation files
            Logger::error("Unable to install Bloom udev rules - \"" + latestBloomRulesPath + "\" does not exist.");
            return;
        }

        std::filesystem::copy(latestBloomRulesPath, bloomRulesPath);
        Logger::warning("Bloom udev rules installed - a reconnect of the debug tool may be required "
            "before the new udev rules come into effect.");
    }
}

void TargetController::shutdown() {
    if (this->getThreadState() == ThreadState::STOPPED) {
        return;
    }

    try {
        Logger::info("Shutting down TargetController");
        this->eventManager.deregisterListener(this->eventListener->getId());
        this->releaseResources();

    } catch (const std::exception& exception) {
        Logger::error("Failed to properly shutdown TargetController. Error: " + std::string(exception.what()));
    }

    this->setThreadStateAndEmitEvent(ThreadState::STOPPED);
}

void TargetController::suspend() {
    if (this->getThreadState() != ThreadState::READY) {
        return;
    }

    Logger::debug("Suspending TargetController");

    try {
        this->releaseResources();

    } catch (const std::exception& exception) {
        Logger::error("Failed to release connected debug tool and target resources. Error: "
            + std::string(exception.what()));
    }

    this->eventListener->deregisterCallbacksForEventType<Events::DebugSessionFinished>();
    this->eventListener->deregisterCallbacksForEventType<Events::ExtractTargetDescriptor>();
    this->eventListener->deregisterCallbacksForEventType<Events::StopTargetExecution>();
    this->eventListener->deregisterCallbacksForEventType<Events::StepTargetExecution>();
    this->eventListener->deregisterCallbacksForEventType<Events::ResumeTargetExecution>();
    this->eventListener->deregisterCallbacksForEventType<Events::RetrieveRegistersFromTarget>();
    this->eventListener->deregisterCallbacksForEventType<Events::WriteRegistersToTarget>();
    this->eventListener->deregisterCallbacksForEventType<Events::RetrieveMemoryFromTarget>();
    this->eventListener->deregisterCallbacksForEventType<Events::WriteMemoryToTarget>();
    this->eventListener->deregisterCallbacksForEventType<Events::SetBreakpointOnTarget>();
    this->eventListener->deregisterCallbacksForEventType<Events::RemoveBreakpointOnTarget>();
    this->eventListener->deregisterCallbacksForEventType<Events::SetProgramCounterOnTarget>();
    this->eventListener->deregisterCallbacksForEventType<Events::InsightThreadStateChanged>();
    this->eventListener->deregisterCallbacksForEventType<Events::RetrieveTargetPinStates>();
    this->eventListener->deregisterCallbacksForEventType<Events::SetTargetPinState>();

    this->lastTargetState = TargetState::UNKNOWN;
    this->cachedTargetDescriptor = std::nullopt;

    this->state = TargetControllerState::SUSPENDED;
    this->eventManager.triggerEvent(
        std::make_shared<TargetControllerStateReported>(this->state)
    );

    Logger::debug("TargetController suspended");
}

void TargetController::resume() {
    this->acquireResources();

    this->eventListener->registerCallbackForEventType<Events::DebugSessionFinished>(
        std::bind(&TargetController::onDebugSessionFinishedEvent, this, std::placeholders::_1)
    );

    this->eventListener->registerCallbackForEventType<Events::ExtractTargetDescriptor>(
        std::bind(&TargetController::onExtractTargetDescriptor, this, std::placeholders::_1)
    );

    this->eventListener->registerCallbackForEventType<Events::StopTargetExecution>(
        std::bind(&TargetController::onStopTargetExecutionEvent, this, std::placeholders::_1)
    );

    this->eventListener->registerCallbackForEventType<Events::StepTargetExecution>(
        std::bind(&TargetController::onStepTargetExecutionEvent, this, std::placeholders::_1)
    );

    this->eventListener->registerCallbackForEventType<Events::ResumeTargetExecution>(
        std::bind(&TargetController::onResumeTargetExecutionEvent, this, std::placeholders::_1)
    );

    this->eventListener->registerCallbackForEventType<Events::RetrieveRegistersFromTarget>(
        std::bind(&TargetController::onReadRegistersEvent, this, std::placeholders::_1)
    );

    this->eventListener->registerCallbackForEventType<Events::WriteRegistersToTarget>(
        std::bind(&TargetController::onWriteRegistersEvent, this, std::placeholders::_1)
    );

    this->eventListener->registerCallbackForEventType<Events::RetrieveMemoryFromTarget>(
        std::bind(&TargetController::onReadMemoryEvent, this, std::placeholders::_1)
    );

    this->eventListener->registerCallbackForEventType<Events::WriteMemoryToTarget>(
        std::bind(&TargetController::onWriteMemoryEvent, this, std::placeholders::_1)
    );

    this->eventListener->registerCallbackForEventType<Events::SetBreakpointOnTarget>(
        std::bind(&TargetController::onSetBreakpointEvent, this, std::placeholders::_1)
    );

    this->eventListener->registerCallbackForEventType<Events::RemoveBreakpointOnTarget>(
        std::bind(&TargetController::onRemoveBreakpointEvent, this, std::placeholders::_1)
    );

    this->eventListener->registerCallbackForEventType<Events::SetProgramCounterOnTarget>(
        std::bind(&TargetController::onSetProgramCounterEvent, this, std::placeholders::_1)
    );

    this->eventListener->registerCallbackForEventType<Events::InsightThreadStateChanged>(
        std::bind(&TargetController::onInsightStateChangedEvent, this, std::placeholders::_1)
    );

    this->eventListener->registerCallbackForEventType<Events::RetrieveTargetPinStates>(
        std::bind(&TargetController::onRetrieveTargetPinStatesEvent, this, std::placeholders::_1)
    );

    this->eventListener->registerCallbackForEventType<Events::SetTargetPinState>(
        std::bind(&TargetController::onSetPinStateEvent, this, std::placeholders::_1)
    );

    Logger::debug("TC resumed");
    this->state = TargetControllerState::ACTIVE;
    this->eventManager.triggerEvent(
        std::make_shared<TargetControllerStateReported>(this->state)
    );

    if (this->target->getState() != TargetState::RUNNING) {
        this->target->run();
    }
}

void TargetController::acquireResources() {
    auto debugToolName = this->environmentConfig.debugToolConfig.name;
    auto targetName = this->environmentConfig.targetConfig.name;

    auto supportedDebugTools = TargetController::getSupportedDebugTools();
    auto supportedTargets = TargetController::getSupportedTargets();

    if (!supportedDebugTools.contains(debugToolName)) {
        throw Exceptions::InvalidConfig(
            "Debug tool name (\"" + debugToolName + "\") not recognised. Please check your configuration!"
        );
    }

    if (!supportedTargets.contains(targetName)) {
        throw Exceptions::InvalidConfig(
            "Target name (\"" + targetName + "\") not recognised. Please check your configuration!"
        );
    }

    // Initiate debug tool and target
    this->debugTool = supportedDebugTools.at(debugToolName)();

    Logger::info("Connecting to debug tool");
    this->debugTool->init();

    Logger::info("Debug tool connected");
    Logger::info("Debug tool name: " + this->debugTool->getName());
    Logger::info("Debug tool serial: " + this->debugTool->getSerialNumber());

    this->target = supportedTargets.at(targetName)();

    if (!this->target->isDebugToolSupported(this->debugTool.get())) {
        throw Exceptions::InvalidConfig(
            "Debug tool (\"" + this->debugTool->getName() + "\") not supported " +
                "by target (\"" + this->target->getName() + "\")."
        );
    }

    this->target->setDebugTool(this->debugTool.get());
    this->target->preActivationConfigure(this->environmentConfig.targetConfig);

    Logger::info("Activating target");
    this->target->activate();
    Logger::info("Target activated");
    this->target->postActivationConfigure();

    while (this->target->supportsPromotion()) {
        auto promotedTarget = this->target->promote();

        if (promotedTarget == nullptr
            || std::type_index(typeid(*promotedTarget)) == std::type_index(typeid(*this->target))
            ) {
            break;
        }

        this->target.reset(promotedTarget.release());
        this->target->postPromotionConfigure();
    }

    Logger::info("Target ID: " + this->target->getHumanReadableId());
    Logger::info("Target name: " + this->target->getName());
}

void TargetController::releaseResources() {
    auto target = this->getTarget();
    auto debugTool = this->getDebugTool();

    if (debugTool != nullptr && debugTool->isInitialised()) {
        if (target != nullptr) {
            /*
             * We call deactivate() without checking if the target is activated. This will address any cases
             * where a target is only partially activated (where the call to activate() failed).
             */
            Logger::info("Deactivating target");
            target->deactivate();
        }

        Logger::info("Closing debug tool");
        debugTool->close();
    }

    this->debugTool.reset();
    this->target.reset();
}

void TargetController::fireTargetEvents() {
    auto newTargetState = this->target->getState();

    if (newTargetState != this->lastTargetState) {
        this->lastTargetState = newTargetState;

        if (newTargetState == TargetState::STOPPED) {
            Logger::debug("Target state changed - STOPPED");
            this->eventManager.triggerEvent(std::make_shared<TargetExecutionStopped>(
                this->target->getProgramCounter(),
                TargetBreakCause::UNKNOWN
            ));
        }

        if (newTargetState == TargetState::RUNNING) {
            Logger::debug("Target state changed - RUNNING");
            this->eventManager.triggerEvent(std::make_shared<TargetExecutionResumed>());
        }
    }
}

void TargetController::emitErrorEvent(int correlationId) {
    auto errorEvent = std::make_shared<Events::TargetControllerErrorOccurred>();
    errorEvent->correlationId = correlationId;
    this->eventManager.triggerEvent(errorEvent);
}

void TargetController::onShutdownTargetControllerEvent(EventPointer<Events::ShutdownTargetController> event) {
    this->shutdown();
}

void TargetController::onStateReportRequest(EventPointer<Events::ReportTargetControllerState> event) {
    auto stateEvent = std::make_shared<TargetControllerStateReported>(this->state);
    stateEvent->correlationId = event->id;
    this->eventManager.triggerEvent(stateEvent);
}

void TargetController::onDebugSessionStartedEvent(EventPointer<Events::DebugSessionStarted>) {
    if (this->state == TargetControllerState::SUSPENDED) {
        Logger::debug("Waking TargetController");

        this->resume();
        this->fireTargetEvents();
    }

    this->target->reset();

    if (this->target->getState() != TargetState::STOPPED) {
        this->target->stop();
    }
}

void TargetController::onDebugSessionFinishedEvent(EventPointer<DebugSessionFinished>) {
    if (this->target->getState() != TargetState::RUNNING) {
        this->target->run();
        this->fireTargetEvents();
    }

    if (this->environmentConfig.debugToolConfig.releasePostDebugSession) {
        this->suspend();
    }
}

void TargetController::onExtractTargetDescriptor(EventPointer<Events::ExtractTargetDescriptor> event) {
    if (!this->cachedTargetDescriptor.has_value()) {
        this->cachedTargetDescriptor = this->target->getDescriptor();
    }

    auto targetDescriptorExtracted = std::make_shared<TargetDescriptorExtracted>();
    targetDescriptorExtracted->targetDescriptor = this->cachedTargetDescriptor.value();

    targetDescriptorExtracted->correlationId = event->id;
    this->eventManager.triggerEvent(targetDescriptorExtracted);
}

void TargetController::onStopTargetExecutionEvent(EventPointer<Events::StopTargetExecution> event) {
    if (this->target->getState() != TargetState::STOPPED) {
        this->target->stop();
        this->lastTargetState = TargetState::STOPPED;
    }

    auto executionStoppedEvent = std::make_shared<TargetExecutionStopped>(
        this->target->getProgramCounter(),
        TargetBreakCause::UNKNOWN
    );

    executionStoppedEvent->correlationId = event->id;
    this->eventManager.triggerEvent(executionStoppedEvent);
}

void TargetController::onStepTargetExecutionEvent(EventPointer<Events::StepTargetExecution> event) {
    try {
        if (this->target->getState() != TargetState::STOPPED) {
            // We can't step the target if it's already running.
            throw Exception("Target is already running");
        }

        if (event->fromProgramCounter.has_value()) {
            this->target->setProgramCounter(event->fromProgramCounter.value());
        }

        this->target->step();
        this->lastTargetState = TargetState::RUNNING;

        auto executionResumedEvent = std::make_shared<TargetExecutionResumed>();
        executionResumedEvent->correlationId = event->id;
        this->eventManager.triggerEvent(executionResumedEvent);

    } catch (const Exception& exception) {
        Logger::error("Failed to step execution on target - " + exception.getMessage());
        this->emitErrorEvent(event->id);
    }
}

void TargetController::onResumeTargetExecutionEvent(EventPointer<Events::ResumeTargetExecution> event) {
    try {
        if (this->target->getState() != TargetState::RUNNING) {
            if (event->fromProgramCounter.has_value()) {
                this->target->setProgramCounter(event->fromProgramCounter.value());
            }

            this->target->run();
            this->lastTargetState = TargetState::RUNNING;
        }

        auto executionResumedEvent = std::make_shared<Events::TargetExecutionResumed>();
        executionResumedEvent->correlationId = event->id;
        this->eventManager.triggerEvent(executionResumedEvent);

    } catch (const Exception& exception) {
        Logger::error("Failed to resume execution on target - " + exception.getMessage());
        this->emitErrorEvent(event->id);
    }
}

void TargetController::onReadRegistersEvent(EventPointer<Events::RetrieveRegistersFromTarget> event) {
    try {
        auto registers = this->target->readRegisters(event->descriptors);

        if (registers.size() > 0) {
            auto registersRetrievedEvent = std::make_shared<Events::RegistersRetrievedFromTarget>();
            registersRetrievedEvent->correlationId = event->id;
            registersRetrievedEvent->registers = registers;
            this->eventManager.triggerEvent(registersRetrievedEvent);
        }

    } catch (const Exception& exception) {
        Logger::error("Failed to read general registers from target - " + exception.getMessage());
        this->emitErrorEvent(event->id);
    }
}

void TargetController::onWriteRegistersEvent(EventPointer<Events::WriteRegistersToTarget> event) {
    try {
        this->target->writeRegisters(event->registers);

        auto registersWrittenEvent = std::make_shared<Events::RegistersWrittenToTarget>();
        registersWrittenEvent->correlationId = event->id;
        this->eventManager.triggerEvent(registersWrittenEvent);

    } catch (const Exception& exception) {
        Logger::error("Failed to write registers to target - " + exception.getMessage());
        this->emitErrorEvent(event->id);
    }
}

void TargetController::onReadMemoryEvent(EventPointer<Events::RetrieveMemoryFromTarget> event) {
    try {
        auto memoryReadEvent = std::make_shared<Events::MemoryRetrievedFromTarget>();
        memoryReadEvent->correlationId = event->id;
        memoryReadEvent->data = this->target->readMemory(event->memoryType, event->startAddress, event->bytes);

        this->eventManager.triggerEvent(memoryReadEvent);

    } catch (const Exception& exception) {
        Logger::error("Failed to read memory from target - " + exception.getMessage());
        this->emitErrorEvent(event->id);
    }
}

void TargetController::onWriteMemoryEvent(EventPointer<Events::WriteMemoryToTarget> event) {
    try {
        this->target->writeMemory(event->memoryType, event->startAddress, event->buffer);

        auto memoryWrittenEvent = std::make_shared<Events::MemoryWrittenToTarget>();
        memoryWrittenEvent->correlationId = event->id;
        this->eventManager.triggerEvent(memoryWrittenEvent);

        if (this->target->memoryAddressRangeClashesWithIoPortRegisters(
            event->memoryType,
            event->startAddress,
            static_cast<std::uint32_t>(event->startAddress + (event->buffer.size() - 1))
        )) {
            // This memory write has affected the target's IO port values
            this->eventManager.triggerEvent(std::make_shared<Events::TargetIoPortsUpdated>());
        }

    } catch (const Exception& exception) {
        Logger::error("Failed to write memory to target - " + exception.getMessage());
        this->emitErrorEvent(event->id);
    }
}

void TargetController::onSetBreakpointEvent(EventPointer<Events::SetBreakpointOnTarget> event) {
    try {
        this->target->setBreakpoint(event->breakpoint.address);
        auto breakpointSetEvent = std::make_shared<Events::BreakpointSetOnTarget>();
        breakpointSetEvent->correlationId = event->id;

        this->eventManager.triggerEvent(breakpointSetEvent);

    } catch (const Exception& exception) {
        Logger::error("Failed to set breakpoint on target - " + exception.getMessage());
        this->emitErrorEvent(event->id);
    }
}

void TargetController::onRemoveBreakpointEvent(EventPointer<Events::RemoveBreakpointOnTarget> event) {
    try {
        this->target->removeBreakpoint(event->breakpoint.address);
        auto breakpointRemovedEvent = std::make_shared<Events::BreakpointRemovedOnTarget>();
        breakpointRemovedEvent->correlationId = event->id;

        this->eventManager.triggerEvent(breakpointRemovedEvent);

    } catch (const Exception& exception) {
        Logger::error("Failed to remove breakpoint on target - " + exception.getMessage());
        this->emitErrorEvent(event->id);
    }
}

void TargetController::onSetProgramCounterEvent(EventPointer<Events::SetProgramCounterOnTarget> event) {
    try {
        if (this->target->getState() != TargetState::STOPPED) {
            throw Exception("Invalid target state - target must be stopped before the program counter can be updated");
        }

        this->target->setProgramCounter(event->address);
        auto programCounterSetEvent = std::make_shared<Events::ProgramCounterSetOnTarget>();
        programCounterSetEvent->correlationId = event->id;

        this->eventManager.triggerEvent(programCounterSetEvent);

    } catch (const Exception& exception) {
        Logger::error("Failed to set program counter on target - " + exception.getMessage());
        this->emitErrorEvent(event->id);
    }
}

// TODO: remove this
void TargetController::onInsightStateChangedEvent(EventPointer<Events::InsightThreadStateChanged> event) {
    if (event->getState() == ThreadState::READY) {
        /*
         * Insight has just started up.
         *
         * Refresh the target state and kick off a target stop/resume execution event. Setting the lastTargetState
         * to UNKNOWN will be enough to do this. See TargetController::fireTargetEvents().
         */
        this->lastTargetState = TargetState::UNKNOWN;
    }
}

void TargetController::onRetrieveTargetPinStatesEvent(EventPointer<Events::RetrieveTargetPinStates> event) {
    try {
        if (this->target->getState() != TargetState::STOPPED) {
            throw Exception("Invalid target state - target must be stopped before pin states can be retrieved");
        }

        auto pinStatesRetrieved = std::make_shared<Events::TargetPinStatesRetrieved>();
        pinStatesRetrieved->correlationId = event->id;
        pinStatesRetrieved->variantId = event->variantId;
        pinStatesRetrieved->pinSatesByNumber = this->target->getPinStates(event->variantId);

        this->eventManager.triggerEvent(pinStatesRetrieved);

    } catch (const Exception& exception) {
        Logger::error("Failed to retrieve target pin states - " + exception.getMessage());
        this->emitErrorEvent(event->id);
    }
}

void TargetController::onSetPinStateEvent(EventPointer<Events::SetTargetPinState> event) {
    try {
        if (this->target->getState() != TargetState::STOPPED) {
            throw Exception("Invalid target state - target must be stopped before pin state can be set");
        }

        this->target->setPinState(event->variantId, event->pinDescriptor, event->pinState);

        auto pinStatesUpdateEvent = std::make_shared<Events::TargetPinStatesRetrieved>();
        pinStatesUpdateEvent->correlationId = event->id;
        pinStatesUpdateEvent->variantId = event->variantId;
        pinStatesUpdateEvent->pinSatesByNumber = {
            {event->pinDescriptor.number, event->pinState}
        };

        this->eventManager.triggerEvent(pinStatesUpdateEvent);

    } catch (const Exception& exception) {
        Logger::error("Failed to set target pin state for pin " + event->pinDescriptor.name + " - "
            + exception.getMessage());
        this->emitErrorEvent(event->id);
    }
}
