#include <thread>
#include <filesystem>
#include <typeindex>

#include "TargetController.hpp"
#include "src/Exceptions/InvalidConfig.hpp"
#include "src/Exceptions/TargetControllerStartupFailure.hpp"
#include "src/Application.hpp"

using namespace Bloom;
using namespace Bloom::Targets;
using namespace Bloom::Events;
using namespace Bloom::Exceptions;

void TargetController::run() {
    try {
        this->startup();

        this->setStateAndEmitEvent(ThreadState::READY);
        Logger::debug("TargetController ready and waiting for events.");

        while (this->getState() == ThreadState::READY) {
            this->fireTargetEvents();
            this->eventListener->waitAndDispatch(60);
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
    this->setState(ThreadState::STARTING);
    this->blockAllSignalsOnCurrentThread();
    this->eventManager.registerListener(this->eventListener);

    // Install Bloom's udev rules if not already installed
    this->checkUdevRules();

    auto debugToolName = this->environmentConfig.debugToolName;
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
    this->setDebugTool(std::move(supportedDebugTools.at(debugToolName)()));
    this->setTarget(supportedTargets.at(targetName)());

    Logger::info("Connecting to debug tool");
    this->debugTool->init();

    Logger::info("Debug tool connected");
    Logger::info("Debug tool name: " + this->debugTool->getName());
    Logger::info("Debug tool serial: " + this->debugTool->getSerialNumber());

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

    if (this->target->getState() == TargetState::STOPPED) {
        this->target->run();
    }

    // Register event handlers
    this->eventListener->registerCallbackForEventType<Events::ShutdownTargetController>(
        std::bind(&TargetController::onShutdownTargetControllerEvent, this, std::placeholders::_1)
    );

    this->eventListener->registerCallbackForEventType<Events::DebugSessionStarted>(
        std::bind(&TargetController::onDebugSessionStartedEvent, this, std::placeholders::_1)
    );

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

    this->eventListener->registerCallbackForEventType<Events::InsightStateChanged>(
        std::bind(&TargetController::onInsightStateChangedEvent, this, std::placeholders::_1)
    );

    this->eventListener->registerCallbackForEventType<Events::RetrieveTargetPinStates>(
        std::bind(&TargetController::onRetrieveTargetPinStatesEvent, this, std::placeholders::_1)
    );

    this->eventListener->registerCallbackForEventType<Events::SetTargetPinState>(
        std::bind(&TargetController::onSetPinStateEvent, this, std::placeholders::_1)
    );
}

void TargetController::checkUdevRules() {
    auto bloomRulesPath = std::string("/etc/udev/rules.d/99-bloom.rules");
    auto latestBloomRulesPath = Application::getResourcesDirPath() + "/UDevRules/99-bloom.rules";

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
    if (this->getState() == ThreadState::STOPPED) {
        return;
    }

    try {
        Logger::info("Shutting down TargetController");
        //this->targetControllerEventListener->clearAllCallbacks();
        this->eventManager.deregisterListener(this->eventListener->getId());
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

    } catch (const std::exception& exception) {
        Logger::error("Failed to properly shutdown TargetController. Error: " + std::string(exception.what()));
    }

    this->setStateAndEmitEvent(ThreadState::STOPPED);
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

void TargetController::onDebugSessionStartedEvent(EventPointer<Events::DebugSessionStarted>) {
    this->target->reset();

    if (this->target->getState() != TargetState::STOPPED) {
        this->target->stop();
    }
}

void TargetController::onDebugSessionFinishedEvent(EventPointer<DebugSessionFinished>) {
    if (this->target->getState() != TargetState::RUNNING) {
        this->target->run();
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
void TargetController::onInsightStateChangedEvent(EventPointer<Events::InsightStateChanged> event) {
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
