#include "TargetControllerComponent.hpp"

#include <thread>
#include <filesystem>
#include <typeindex>
#include <algorithm>

#include "src/Application.hpp"
#include "src/Helpers/Paths.hpp"
#include "src/Logger/Logger.hpp"

#include "src/TargetController/Exceptions/DeviceFailure.hpp"
#include "src/TargetController/Exceptions/TargetOperationFailure.hpp"
#include "src/Exceptions/TargetControllerStartupFailure.hpp"
#include "src/Exceptions/InvalidConfig.hpp"

namespace Bloom::TargetController
{
    using namespace Bloom::Targets;
    using namespace Bloom::Events;
    using namespace Bloom::Exceptions;

    using Commands::Command;
    using Commands::CommandIdType;
    using Commands::StopTargetExecution;
    using Commands::ResumeTargetExecution;
    using Commands::ResetTarget;
    using Commands::ReadTargetRegisters;
    using Commands::WriteTargetRegisters;

    using Responses::Response;
    using Responses::TargetRegistersRead;

    TargetControllerComponent::TargetControllerComponent(
        const ProjectConfig& projectConfig,
        const EnvironmentConfig& environmentConfig
    )
        : projectConfig(projectConfig)
        , environmentConfig(environmentConfig)
    {}

    void TargetControllerComponent::run() {
        try {
            this->startup();

            this->setThreadStateAndEmitEvent(ThreadState::READY);
            Logger::debug("TargetController ready and waiting for events.");

            while (this->getThreadState() == ThreadState::READY) {
                try {
                    if (this->state == TargetControllerState::ACTIVE) {
                        this->fireTargetEvents();
                    }

                    TargetControllerComponent::notifier.waitForNotification(std::chrono::milliseconds(60));

                    this->processQueuedCommands();
                    this->eventListener->dispatchCurrentEvents();

                } catch (const DeviceFailure& exception) {
                    /*
                     * Upon a device failure, we assume Bloom has lost control of the debug tool. This could be the
                     * result of the user disconnecting the debug tool, or issuing a soft reset. The soft reset could
                     * have been issued via another application, without the user's knowledge.
                     * See https://github.com/navnavnav/Bloom/issues/3 for more on that.
                     *
                     * The TC will go into a suspended state and the DebugServer should terminate any active debug
                     * session. When the user attempts to start another debug session, we will try to re-connect to the
                     * debug tool.
                     */
                    Logger::error("Device failure detected - " + exception.getMessage());
                    Logger::error("Suspending TargetController");
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

    void TargetControllerComponent::registerCommand(std::unique_ptr<Command> command) {
        auto commandQueueLock = TargetControllerComponent::commandQueue.acquireLock();
        TargetControllerComponent::commandQueue.getValue().push(std::move(command));
        TargetControllerComponent::notifier.notify();
    }

    std::optional<std::unique_ptr<Responses::Response>> TargetControllerComponent::waitForResponse(
        CommandIdType commandId,
        std::optional<std::chrono::milliseconds> timeout
    ) {
        auto response = std::unique_ptr<Response>(nullptr);

        const auto predicate = [commandId, &response] {
            auto& responsesByCommandId = TargetControllerComponent::responsesByCommandId.getValue();
            auto responseIt = responsesByCommandId.find(commandId);

            if (responseIt != responsesByCommandId.end()) {
                response.swap(responseIt->second);
                responsesByCommandId.erase(responseIt);

                return true;
            }

            return false;
        };

        auto responsesByCommandIdLock = TargetControllerComponent::responsesByCommandId.acquireLock();

        if (timeout.has_value()) {
            TargetControllerComponent::responsesByCommandIdCv.wait_for(
                responsesByCommandIdLock,
                timeout.value(),
                predicate
            );

        } else {
            TargetControllerComponent::responsesByCommandIdCv.wait(responsesByCommandIdLock, predicate);
        }

        return (response != nullptr) ? std::optional(std::move(response)) : std::nullopt;
    }

    void TargetControllerComponent::deregisterCommandHandler(Commands::CommandType commandType) {
        this->commandHandlersByCommandType.erase(commandType);
    }

    void TargetControllerComponent::startup() {
        this->setName("TC");
        Logger::info("Starting TargetController");
        this->setThreadState(ThreadState::STARTING);
        this->blockAllSignalsOnCurrentThread();
        EventManager::registerListener(this->eventListener);

        // Install Bloom's udev rules if not already installed
        TargetControllerComponent::checkUdevRules();

        // Register event handlers
        this->eventListener->registerCallbackForEventType<Events::ReportTargetControllerState>(
            std::bind(&TargetControllerComponent::onStateReportRequest, this, std::placeholders::_1)
        );

        this->eventListener->registerCallbackForEventType<Events::ShutdownTargetController>(
            std::bind(&TargetControllerComponent::onShutdownTargetControllerEvent, this, std::placeholders::_1)
        );

        this->eventListener->registerCallbackForEventType<Events::DebugSessionStarted>(
            std::bind(&TargetControllerComponent::onDebugSessionStartedEvent, this, std::placeholders::_1)
        );

        this->resume();
    }

    std::map<
        std::string,
        std::function<std::unique_ptr<DebugTool>()>
    > TargetControllerComponent::getSupportedDebugTools() {
        return std::map<std::string, std::function<std::unique_ptr<DebugTool>()>> {
            {
                "atmel-ice",
                [] {
                    return std::make_unique<DebugToolDrivers::AtmelIce>();
                }
            },
            {
                "power-debugger",
                [] {
                    return std::make_unique<DebugToolDrivers::PowerDebugger>();
                }
            },
            {
                "snap",
                [] {
                    return std::make_unique<DebugToolDrivers::MplabSnap>();
                }
            },
            {
                "pickit-4",
                [] {
                    return std::make_unique<DebugToolDrivers::MplabPickit4>();
                }
            },
            {
                "xplained-pro",
                [] {
                    return std::make_unique<DebugToolDrivers::XplainedPro>();
                }
            },
            {
                "xplained-mini",
                [] {
                    return std::make_unique<DebugToolDrivers::XplainedMini>();
                }
            },
            {
                "xplained-nano",
                [] {
                    return std::make_unique<DebugToolDrivers::XplainedNano>();
                }
            },
            {
                "curiosity-nano",
                [] {
                    return std::make_unique<DebugToolDrivers::CuriosityNano>();
                }
            },
        };
    }

    std::map<
        std::string,
        std::function<std::unique_ptr<Targets::Target>()>
    > TargetControllerComponent::getSupportedTargets() {
        using Avr8TargetDescriptionFile = Targets::Microchip::Avr::Avr8Bit::TargetDescription::TargetDescriptionFile;

        auto mapping = std::map<std::string, std::function<std::unique_ptr<Targets::Target>()>>({
            {
                "avr8",
                [] {
                    return std::make_unique<Targets::Microchip::Avr::Avr8Bit::Avr8>();
                }
            },
        });

        // Include all targets from AVR8 target description files
        auto avr8PdMapping = Avr8TargetDescriptionFile::getTargetDescriptionMapping();

        for (auto mapIt = avr8PdMapping.begin(); mapIt != avr8PdMapping.end(); mapIt++) {
            // Each target signature maps to an array of targets, as numerous targets can possess the same signature.
            auto targets = mapIt.value().toArray();

            for (auto targetIt = targets.begin(); targetIt != targets.end(); targetIt++) {
                auto targetName = targetIt->toObject().find("targetName").value().toString()
                    .toLower().toStdString();
                auto targetSignatureHex = mapIt.key().toLower().toStdString();

                if (!mapping.contains(targetName)) {
                    mapping.insert({
                        targetName,
                        [targetName, targetSignatureHex] {
                            return std::make_unique<Targets::Microchip::Avr::Avr8Bit::Avr8>(
                                targetName,
                                Targets::Microchip::Avr::TargetSignature(targetSignatureHex)
                            );
                        }
                    });
                }
            }
        }

        return mapping;
    }

    void TargetControllerComponent::processQueuedCommands() {
        auto commands = std::queue<std::unique_ptr<Command>>();

        {
            auto queueLock = TargetControllerComponent::commandQueue.acquireLock();
            commands.swap(TargetControllerComponent::commandQueue.getValue());
        }

        while (!commands.empty()) {
            const auto command = std::move(commands.front());
            commands.pop();

            const auto commandId = command->id;
            const auto commandType = command->getType();

            try {
                if (!this->commandHandlersByCommandType.contains(commandType)) {
                    throw Exception("No handler registered for this command.");
                }

                this->registerCommandResponse(
                    commandId,
                    this->commandHandlersByCommandType.at(commandType)(*(command.get()))
                );

            } catch (const Exception& exception) {
                this->registerCommandResponse(
                    commandId,
                    std::make_unique<Responses::Error>(exception.getMessage())
                );
            }
        }
    }

    void TargetControllerComponent::registerCommandResponse(
        CommandIdType commandId,
        std::unique_ptr<Response> response
    ) {
        auto responseMappingLock = TargetControllerComponent::responsesByCommandId.acquireLock();
        TargetControllerComponent::responsesByCommandId.getValue().insert(
            std::pair(commandId, std::move(response))
        );
        TargetControllerComponent::responsesByCommandIdCv.notify_all();
    }

    void TargetControllerComponent::checkUdevRules() {
        auto bloomRulesPath = std::string("/etc/udev/rules.d/99-bloom.rules");
        auto latestBloomRulesPath = Paths::resourcesDirPath() + "/UDevRules/99-bloom.rules";

        if (!std::filesystem::exists(bloomRulesPath)) {
            Logger::warning("Bloom udev rules missing - attempting installation");

            // We can only install them if we're running as root
            if (!Application::isRunningAsRoot()) {
                Logger::error("Bloom udev rules missing - cannot install udev rules without root privileges.\n"
                    "Running Bloom once with root privileges will allow it to automatically install the udev rules. "
                    "Alternatively, instructions on manually installing the udev rules can be found "
                    "here: " + Paths::homeDomainName() + "/docs/getting-started\nBloom may fail to connect to some "
                    "debug tools until this is resolved.");
                return;
            }

            if (!std::filesystem::exists(latestBloomRulesPath)) {
                // This shouldn't happen, but it can if someone has been messing with the installation files
                Logger::error(
                    "Unable to install Bloom udev rules - \"" + latestBloomRulesPath + "\" does not exist."
                );
                return;
            }

            std::filesystem::copy(latestBloomRulesPath, bloomRulesPath);
            Logger::warning("Bloom udev rules installed - a reconnect of the debug tool may be required "
                "before the new udev rules come into effect.");
        }
    }

    void TargetControllerComponent::shutdown() {
        if (this->getThreadState() == ThreadState::STOPPED) {
            return;
        }

        try {
            Logger::info("Shutting down TargetController");
            EventManager::deregisterListener(this->eventListener->getId());
            this->releaseHardware();

        } catch (const std::exception& exception) {
            this->target.reset();
            this->debugTool.reset();
            Logger::error(
                "Failed to properly shutdown TargetController. Error: " + std::string(exception.what())
            );
        }

        this->setThreadStateAndEmitEvent(ThreadState::STOPPED);
    }

    void TargetControllerComponent::suspend() {
        if (this->getThreadState() != ThreadState::READY) {
            return;
        }

        Logger::debug("Suspending TargetController");

        try {
            this->releaseHardware();

        } catch (const std::exception& exception) {
            Logger::error("Failed to release connected debug tool and target resources. Error: "
                + std::string(exception.what()));
        }

        this->deregisterCommandHandler(StopTargetExecution::type);
        this->deregisterCommandHandler(ResumeTargetExecution::type);
        this->deregisterCommandHandler(ResetTarget::type);
        this->deregisterCommandHandler(ReadTargetRegisters::type);
        this->deregisterCommandHandler(WriteTargetRegisters::type);

        this->eventListener->deregisterCallbacksForEventType<Events::DebugSessionFinished>();
        this->eventListener->deregisterCallbacksForEventType<Events::ExtractTargetDescriptor>();
        this->eventListener->deregisterCallbacksForEventType<Events::StepTargetExecution>();
        this->eventListener->deregisterCallbacksForEventType<Events::RetrieveMemoryFromTarget>();
        this->eventListener->deregisterCallbacksForEventType<Events::WriteMemoryToTarget>();
        this->eventListener->deregisterCallbacksForEventType<Events::SetBreakpointOnTarget>();
        this->eventListener->deregisterCallbacksForEventType<Events::RemoveBreakpointOnTarget>();
        this->eventListener->deregisterCallbacksForEventType<Events::SetProgramCounterOnTarget>();
        this->eventListener->deregisterCallbacksForEventType<Events::InsightThreadStateChanged>();
        this->eventListener->deregisterCallbacksForEventType<Events::RetrieveTargetPinStates>();
        this->eventListener->deregisterCallbacksForEventType<Events::SetTargetPinState>();
        this->eventListener->deregisterCallbacksForEventType<Events::RetrieveStackPointerFromTarget>();

        this->lastTargetState = TargetState::UNKNOWN;
        this->cachedTargetDescriptor = std::nullopt;
        this->registerDescriptorsByMemoryType.clear();
        this->registerAddressRangeByMemoryType.clear();

        this->state = TargetControllerState::SUSPENDED;
        EventManager::triggerEvent(std::make_shared<TargetControllerStateReported>(this->state));

        Logger::debug("TargetController suspended");
    }

    void TargetControllerComponent::resume() {
        this->acquireHardware();
        this->loadRegisterDescriptors();

        this->registerCommandHandler<StopTargetExecution>(
            std::bind(&TargetControllerComponent::handleStopTargetExecution, this, std::placeholders::_1)
        );

        this->registerCommandHandler<ResumeTargetExecution>(
            std::bind(&TargetControllerComponent::handleResumeTargetExecution, this, std::placeholders::_1)
        );

        this->registerCommandHandler<ResetTarget>(
            std::bind(&TargetControllerComponent::handleResetTarget, this, std::placeholders::_1)
        );

        this->registerCommandHandler<ReadTargetRegisters>(
            std::bind(&TargetControllerComponent::handleReadTargetRegisters, this, std::placeholders::_1)
        );

        this->registerCommandHandler<WriteTargetRegisters>(
            std::bind(&TargetControllerComponent::handleWriteTargetRegisters, this, std::placeholders::_1)
        );

        this->eventListener->registerCallbackForEventType<Events::DebugSessionFinished>(
            std::bind(&TargetControllerComponent::onDebugSessionFinishedEvent, this, std::placeholders::_1)
        );

        this->eventListener->registerCallbackForEventType<Events::ExtractTargetDescriptor>(
            std::bind(&TargetControllerComponent::onExtractTargetDescriptor, this, std::placeholders::_1)
        );

        this->eventListener->registerCallbackForEventType<Events::StepTargetExecution>(
            std::bind(&TargetControllerComponent::onStepTargetExecutionEvent, this, std::placeholders::_1)
        );

        this->eventListener->registerCallbackForEventType<Events::RetrieveMemoryFromTarget>(
            std::bind(&TargetControllerComponent::onReadMemoryEvent, this, std::placeholders::_1)
        );

        this->eventListener->registerCallbackForEventType<Events::WriteMemoryToTarget>(
            std::bind(&TargetControllerComponent::onWriteMemoryEvent, this, std::placeholders::_1)
        );

        this->eventListener->registerCallbackForEventType<Events::SetBreakpointOnTarget>(
            std::bind(&TargetControllerComponent::onSetBreakpointEvent, this, std::placeholders::_1)
        );

        this->eventListener->registerCallbackForEventType<Events::RemoveBreakpointOnTarget>(
            std::bind(&TargetControllerComponent::onRemoveBreakpointEvent, this, std::placeholders::_1)
        );

        this->eventListener->registerCallbackForEventType<Events::SetProgramCounterOnTarget>(
            std::bind(&TargetControllerComponent::onSetProgramCounterEvent, this, std::placeholders::_1)
        );

        this->eventListener->registerCallbackForEventType<Events::InsightThreadStateChanged>(
            std::bind(&TargetControllerComponent::onInsightStateChangedEvent, this, std::placeholders::_1)
        );

        this->eventListener->registerCallbackForEventType<Events::RetrieveTargetPinStates>(
            std::bind(&TargetControllerComponent::onRetrieveTargetPinStatesEvent, this, std::placeholders::_1)
        );

        this->eventListener->registerCallbackForEventType<Events::SetTargetPinState>(
            std::bind(&TargetControllerComponent::onSetPinStateEvent, this, std::placeholders::_1)
        );

        this->eventListener->registerCallbackForEventType<Events::RetrieveStackPointerFromTarget>(
            std::bind(&TargetControllerComponent::onRetrieveStackPointerEvent, this, std::placeholders::_1)
        );

        this->state = TargetControllerState::ACTIVE;
        EventManager::triggerEvent(
            std::make_shared<TargetControllerStateReported>(this->state)
        );

        if (this->target->getState() != TargetState::RUNNING) {
            this->target->run();
        }
    }

    void TargetControllerComponent::acquireHardware() {
        auto debugToolName = this->environmentConfig.debugToolConfig.name;
        auto targetName = this->environmentConfig.targetConfig.name;

        static auto supportedDebugTools = this->getSupportedDebugTools();
        static auto supportedTargets = this->getSupportedTargets();

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

            this->target = std::move(promotedTarget);
            this->target->postPromotionConfigure();
        }

        Logger::info("Target ID: " + this->target->getHumanReadableId());
        Logger::info("Target name: " + this->target->getName());
    }

    void TargetControllerComponent::releaseHardware() {
        /*
         * Transferring ownership of this->debugTool and this->target to this function block means if an exception is
         * thrown, the objects will still be destroyed.
         */
        auto debugTool = std::move(this->debugTool);
        auto target = std::move(this->target);

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
    }

    void TargetControllerComponent::loadRegisterDescriptors() {
        auto& targetDescriptor = this->getTargetDescriptor();

        for (const auto& [registerType, registerDescriptors] : targetDescriptor.registerDescriptorsByType) {
            for (const auto& registerDescriptor : registerDescriptors) {
                auto startAddress = registerDescriptor.startAddress.value_or(0);
                auto endAddress = startAddress + (registerDescriptor.size - 1);

                if (!this->registerAddressRangeByMemoryType.contains(registerDescriptor.memoryType)) {
                    auto addressRange = TargetMemoryAddressRange();
                    addressRange.startAddress = startAddress;
                    addressRange.endAddress = endAddress;
                    this->registerAddressRangeByMemoryType.insert(
                        std::pair(registerDescriptor.memoryType, addressRange)
                    );

                } else {
                    auto& addressRange = this->registerAddressRangeByMemoryType.at(registerDescriptor.memoryType);

                    if (startAddress < addressRange.startAddress) {
                        addressRange.startAddress = startAddress;
                    }

                    if (endAddress > addressRange.endAddress) {
                        addressRange.endAddress = endAddress;
                    }
                }

                this->registerDescriptorsByMemoryType[registerDescriptor.memoryType].insert(registerDescriptor);
            }
        }
    }

    TargetRegisterDescriptors TargetControllerComponent::getRegisterDescriptorsWithinAddressRange(
        std::uint32_t startAddress,
        std::uint32_t endAddress,
        Targets::TargetMemoryType memoryType
    ) {
        auto output = TargetRegisterDescriptors();

        if (this->registerAddressRangeByMemoryType.contains(memoryType)
            && this->registerDescriptorsByMemoryType.contains(memoryType)
        ) {
            auto& registersAddressRange = this->registerAddressRangeByMemoryType.at(memoryType);

            if (
                (startAddress <= registersAddressRange.startAddress && endAddress >= registersAddressRange.startAddress)
                || (startAddress <= registersAddressRange.endAddress && endAddress >= registersAddressRange.startAddress)
            ) {
                auto& registerDescriptors = this->registerDescriptorsByMemoryType.at(memoryType);

                for (const auto& registerDescriptor : registerDescriptors) {
                    if (!registerDescriptor.startAddress.has_value() || registerDescriptor.size < 1) {
                        continue;
                    }

                    auto registerStartAddress = registerDescriptor.startAddress.value();
                    auto registerEndAddress = registerStartAddress + registerDescriptor.size;

                    if (
                        (startAddress <= registerStartAddress && endAddress >= registerStartAddress)
                        || (startAddress <= registerEndAddress && endAddress >= registerStartAddress)
                    ) {
                        output.insert(registerDescriptor);
                    }
                }
            }
        }

        return output;
    }

    void TargetControllerComponent::fireTargetEvents() {
        auto newTargetState = this->target->getState();

        if (newTargetState != this->lastTargetState) {
            this->lastTargetState = newTargetState;

            if (newTargetState == TargetState::STOPPED) {
                Logger::debug("Target state changed - STOPPED");
                EventManager::triggerEvent(std::make_shared<TargetExecutionStopped>(
                    this->target->getProgramCounter(),
                    TargetBreakCause::UNKNOWN
                ));
            }

            if (newTargetState == TargetState::RUNNING) {
                Logger::debug("Target state changed - RUNNING");
                EventManager::triggerEvent(std::make_shared<TargetExecutionResumed>());
            }
        }
    }

    void TargetControllerComponent::resetTarget() {
        this->target->reset();

        EventManager::triggerEvent(std::make_shared<Events::TargetReset>());
    }

    void TargetControllerComponent::emitErrorEvent(int correlationId, const std::string& errorMessage) {
        auto errorEvent = std::make_shared<Events::TargetControllerErrorOccurred>();
        errorEvent->correlationId = correlationId;
        errorEvent->errorMessage = errorMessage;
        EventManager::triggerEvent(errorEvent);
    }

    Targets::TargetDescriptor& TargetControllerComponent::getTargetDescriptor() {
        if (!this->cachedTargetDescriptor.has_value()) {
            this->cachedTargetDescriptor = this->target->getDescriptor();
        }

        return this->cachedTargetDescriptor.value();
    }

    void TargetControllerComponent::onShutdownTargetControllerEvent(const Events::ShutdownTargetController&) {
        this->shutdown();
    }

    void TargetControllerComponent::onStateReportRequest(const Events::ReportTargetControllerState& event) {
        auto stateEvent = std::make_shared<Events::TargetControllerStateReported>(this->state);
        stateEvent->correlationId = event.id;
        EventManager::triggerEvent(stateEvent);
    }

    void TargetControllerComponent::onExtractTargetDescriptor(const Events::ExtractTargetDescriptor& event) {
        auto targetDescriptorExtracted = std::make_shared<TargetDescriptorExtracted>();
        targetDescriptorExtracted->targetDescriptor = this->getTargetDescriptor();

        targetDescriptorExtracted->correlationId = event.id;
        EventManager::triggerEvent(targetDescriptorExtracted);
    }

    void TargetControllerComponent::onDebugSessionStartedEvent(const Events::DebugSessionStarted&) {
        if (this->state == TargetControllerState::SUSPENDED) {
            Logger::debug("Waking TargetController");

            this->resume();
            this->fireTargetEvents();
        }

        this->resetTarget();

        if (this->target->getState() != TargetState::STOPPED) {
            this->target->stop();
        }
    }

    void TargetControllerComponent::onDebugSessionFinishedEvent(const DebugSessionFinished&) {
        if (this->target->getState() != TargetState::RUNNING) {
            this->target->run();
            this->fireTargetEvents();
        }

        if (this->environmentConfig.debugToolConfig.releasePostDebugSession) {
            this->suspend();
        }
    }

    std::unique_ptr<Response> TargetControllerComponent::handleStopTargetExecution(StopTargetExecution& command) {
        if (this->target->getState() != TargetState::STOPPED) {
            this->target->stop();
            this->lastTargetState = TargetState::STOPPED;
        }

        EventManager::triggerEvent(std::make_shared<Events::TargetExecutionStopped>(
            this->target->getProgramCounter(),
            TargetBreakCause::UNKNOWN
        ));

        return std::make_unique<Response>();
    }

    std::unique_ptr<Response> TargetControllerComponent::handleResumeTargetExecution(
        ResumeTargetExecution& command
    ) {
        if (this->target->getState() != TargetState::RUNNING) {
            if (command.fromProgramCounter.has_value()) {
                this->target->setProgramCounter(command.fromProgramCounter.value());
            }

            this->target->run();
            this->lastTargetState = TargetState::RUNNING;
        }

        EventManager::triggerEvent(std::make_shared<Events::TargetExecutionResumed>());

        return std::make_unique<Response>();
    }

    std::unique_ptr<Response> TargetControllerComponent::handleResetTarget(ResetTarget& command) {
        this->resetTarget();
        return std::make_unique<Response>();
    }

    std::unique_ptr<TargetRegistersRead> TargetControllerComponent::handleReadTargetRegisters(
        ReadTargetRegisters& command
    ) {
        return std::make_unique<TargetRegistersRead>(this->target->readRegisters(command.descriptors));
    }

    std::unique_ptr<Response> TargetControllerComponent::handleWriteTargetRegisters(WriteTargetRegisters& command) {
        this->target->writeRegisters(command.registers);

        auto registersWrittenEvent = std::make_shared<Events::RegistersWrittenToTarget>();
        registersWrittenEvent->registers = command.registers;

        EventManager::triggerEvent(registersWrittenEvent);

        return std::make_unique<Response>();
    }

    void TargetControllerComponent::onStepTargetExecutionEvent(const Events::StepTargetExecution& event) {
        try {
            if (this->target->getState() != TargetState::STOPPED) {
                // We can't step the target if it's already running.
                throw TargetOperationFailure("Target is already running");
            }

            if (event.fromProgramCounter.has_value()) {
                this->target->setProgramCounter(event.fromProgramCounter.value());
            }

            this->target->step();
            this->lastTargetState = TargetState::RUNNING;

            auto executionResumedEvent = std::make_shared<Events::TargetExecutionResumed>();
            executionResumedEvent->correlationId = event.id;
            EventManager::triggerEvent(executionResumedEvent);

        } catch (const TargetOperationFailure& exception) {
            Logger::error("Failed to step execution on target - " + exception.getMessage());
            this->emitErrorEvent(event.id, exception.getMessage());
        }
    }

    void TargetControllerComponent::onReadMemoryEvent(const Events::RetrieveMemoryFromTarget& event) {
        try {
            auto memoryReadEvent = std::make_shared<Events::MemoryRetrievedFromTarget>();
            memoryReadEvent->correlationId = event.id;
            memoryReadEvent->data = this->target->readMemory(
                event.memoryType,
                event.startAddress,
                event.bytes,
                event.excludedAddressRanges
            );

            EventManager::triggerEvent(memoryReadEvent);

        } catch (const TargetOperationFailure& exception) {
            Logger::error("Failed to read memory from target - " + exception.getMessage());
            this->emitErrorEvent(event.id, exception.getMessage());
        }
    }

    void TargetControllerComponent::onWriteMemoryEvent(const Events::WriteMemoryToTarget& event) {
        try {
            const auto& buffer = event.buffer;
            const auto bufferSize = event.buffer.size();
            const auto bufferStartAddress = event.startAddress;

            this->target->writeMemory(event.memoryType, event.startAddress, event.buffer);

            auto memoryWrittenEvent = std::make_shared<Events::MemoryWrittenToTarget>();
            memoryWrittenEvent->correlationId = event.id;
            EventManager::triggerEvent(memoryWrittenEvent);

            if (EventManager::isEventTypeListenedFor(Events::RegistersWrittenToTarget::type)
                && this->registerDescriptorsByMemoryType.contains(event.memoryType)
            ) {
                /*
                 * The memory type we just wrote to contains some number of registers - if we've written to any address
                 * that is known to store the value of a register, trigger a RegistersWrittenToTarget event
                 */
                const auto bufferEndAddress = static_cast<std::uint32_t>(bufferStartAddress + (bufferSize - 1));
                auto registerDescriptors = this->getRegisterDescriptorsWithinAddressRange(
                    bufferStartAddress,
                    bufferEndAddress,
                    event.memoryType
                );

                if (!registerDescriptors.empty()) {
                    auto registersWrittenEvent = std::make_shared<Events::RegistersWrittenToTarget>();
                    registersWrittenEvent->correlationId = event.id;

                    for (const auto& registerDescriptor : registerDescriptors) {
                        const auto registerSize = registerDescriptor.size;
                        const auto registerStartAddress = registerDescriptor.startAddress.value();
                        const auto registerEndAddress = registerStartAddress + (registerSize - 1);

                        if (registerStartAddress < bufferStartAddress || registerEndAddress > bufferEndAddress) {
                            continue;
                        }

                        const auto bufferBeginIt = buffer.begin() + (registerStartAddress - bufferStartAddress);
                        registersWrittenEvent->registers.emplace_back(TargetRegister(
                            registerDescriptor,
                            TargetMemoryBuffer(bufferBeginIt, bufferBeginIt + registerSize)
                        ));
                    }

                    EventManager::triggerEvent(registersWrittenEvent);
                }
            }

        } catch (const TargetOperationFailure& exception) {
            Logger::error("Failed to write memory to target - " + exception.getMessage());
            this->emitErrorEvent(event.id, exception.getMessage());
        }
    }

    void TargetControllerComponent::onSetBreakpointEvent(const Events::SetBreakpointOnTarget& event) {
        try {
            this->target->setBreakpoint(event.breakpoint.address);
            auto breakpointSetEvent = std::make_shared<Events::BreakpointSetOnTarget>();
            breakpointSetEvent->correlationId = event.id;

            EventManager::triggerEvent(breakpointSetEvent);

        } catch (const TargetOperationFailure& exception) {
            Logger::error("Failed to set breakpoint on target - " + exception.getMessage());
            this->emitErrorEvent(event.id, exception.getMessage());
        }
    }

    void TargetControllerComponent::onRemoveBreakpointEvent(const Events::RemoveBreakpointOnTarget& event) {
        try {
            this->target->removeBreakpoint(event.breakpoint.address);
            auto breakpointRemovedEvent = std::make_shared<Events::BreakpointRemovedOnTarget>();
            breakpointRemovedEvent->correlationId = event.id;

            EventManager::triggerEvent(breakpointRemovedEvent);

        } catch (const TargetOperationFailure& exception) {
            Logger::error("Failed to remove breakpoint on target - " + exception.getMessage());
            this->emitErrorEvent(event.id, exception.getMessage());
        }
    }

    void TargetControllerComponent::onSetProgramCounterEvent(const Events::SetProgramCounterOnTarget& event) {
        try {
            if (this->target->getState() != TargetState::STOPPED) {
                throw TargetOperationFailure(
                    "Invalid target state - target must be stopped before the program counter can be updated"
                );
            }

            this->target->setProgramCounter(event.address);
            auto programCounterSetEvent = std::make_shared<Events::ProgramCounterSetOnTarget>();
            programCounterSetEvent->correlationId = event.id;

            EventManager::triggerEvent(programCounterSetEvent);

        } catch (const TargetOperationFailure& exception) {
            Logger::error("Failed to set program counter on target - " + exception.getMessage());
            this->emitErrorEvent(event.id, exception.getMessage());
        }
    }

    // TODO: remove this
    void TargetControllerComponent::onInsightStateChangedEvent(const Events::InsightThreadStateChanged& event) {
        if (event.getState() == ThreadState::READY) {
            /*
             * Insight has just started up.
             *
             * Refresh the target state and kick off a target stop/resume execution event. Setting the lastTargetState
             * to UNKNOWN will be enough to do this. See TargetControllerComponent::fireTargetEvents().
             */
            this->lastTargetState = TargetState::UNKNOWN;
        }
    }

    void TargetControllerComponent::onRetrieveTargetPinStatesEvent(const Events::RetrieveTargetPinStates& event) {
        try {
            if (this->target->getState() != TargetState::STOPPED) {
                throw TargetOperationFailure(
                    "Invalid target state - target must be stopped before pin states can be retrieved"
                );
            }

            auto pinStatesRetrieved = std::make_shared<Events::TargetPinStatesRetrieved>();
            pinStatesRetrieved->correlationId = event.id;
            pinStatesRetrieved->variantId = event.variantId;
            pinStatesRetrieved->pinSatesByNumber = this->target->getPinStates(event.variantId);

            EventManager::triggerEvent(pinStatesRetrieved);

        } catch (const TargetOperationFailure& exception) {
            Logger::error("Failed to retrieve target pin states - " + exception.getMessage());
            this->emitErrorEvent(event.id, exception.getMessage());
        }
    }

    void TargetControllerComponent::onSetPinStateEvent(const Events::SetTargetPinState& event) {
        try {
            if (this->target->getState() != TargetState::STOPPED) {
                throw TargetOperationFailure(
                    "Invalid target state - target must be stopped before pin state can be set"
                );
            }

            this->target->setPinState(event.pinDescriptor, event.pinState);

            auto pinStatesUpdateEvent = std::make_shared<Events::TargetPinStatesRetrieved>();
            pinStatesUpdateEvent->correlationId = event.id;
            pinStatesUpdateEvent->variantId = event.pinDescriptor.variantId;
            pinStatesUpdateEvent->pinSatesByNumber = {
                {event.pinDescriptor.number, event.pinState}
            };

            EventManager::triggerEvent(pinStatesUpdateEvent);

        } catch (const TargetOperationFailure& exception) {
            Logger::error("Failed to set target pin state for pin " + event.pinDescriptor.name + " - "
                + exception.getMessage());
            this->emitErrorEvent(event.id, exception.getMessage());
        }
    }

    void TargetControllerComponent::onRetrieveStackPointerEvent(const Events::RetrieveStackPointerFromTarget& event) {
        try {
            if (this->target->getState() != TargetState::STOPPED) {
                throw TargetOperationFailure(
                    "Invalid target state - target must be stopped before stack pointer can be retrieved"
                );
            }

            auto stackPointerRetrieved = std::make_shared<Events::StackPointerRetrievedFromTarget>();
            stackPointerRetrieved->correlationId = event.id;
            stackPointerRetrieved->stackPointer = this->target->getStackPointer();

            EventManager::triggerEvent(stackPointerRetrieved);

        } catch (const TargetOperationFailure& exception) {
            Logger::error("Failed to retrieve stack pointer value from target - " + exception.getMessage());
            this->emitErrorEvent(event.id, exception.getMessage());
        }
    }
}
