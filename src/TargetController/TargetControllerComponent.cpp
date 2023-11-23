#include "TargetControllerComponent.hpp"

#include <filesystem>
#include <typeindex>
#include <algorithm>

#include "Responses/Error.hpp"

#include "src/Services/ProcessService.hpp"
#include "src/Services/StringService.hpp"
#include "src/Logger/Logger.hpp"

#include "src/Exceptions/InvalidConfig.hpp"

namespace TargetController
{
    using namespace Targets;
    using namespace Events;
    using namespace Exceptions;

    using Commands::CommandIdType;

    using Commands::Command;
    using Commands::StartAtomicSession;
    using Commands::EndAtomicSession;
    using Commands::Shutdown;
    using Commands::GetTargetDescriptor;
    using Commands::GetTargetState;
    using Commands::StopTargetExecution;
    using Commands::ResumeTargetExecution;
    using Commands::ResetTarget;
    using Commands::ReadTargetRegisters;
    using Commands::WriteTargetRegisters;
    using Commands::ReadTargetMemory;
    using Commands::WriteTargetMemory;
    using Commands::EraseTargetMemory;
    using Commands::StepTargetExecution;
    using Commands::SetBreakpoint;
    using Commands::RemoveBreakpoint;
    using Commands::SetTargetProgramCounter;
    using Commands::GetTargetPinStates;
    using Commands::SetTargetPinState;
    using Commands::GetTargetStackPointer;
    using Commands::GetTargetProgramCounter;
    using Commands::EnableProgrammingMode;
    using Commands::DisableProgrammingMode;

    using Responses::Response;
    using Responses::AtomicSessionId;
    using Responses::TargetRegistersRead;
    using Responses::TargetMemoryRead;
    using Responses::TargetPinStates;
    using Responses::TargetStackPointer;
    using Responses::TargetProgramCounter;
    using Responses::Breakpoint;

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
            Logger::debug("TargetController ready");

            while (this->getThreadState() == ThreadState::READY) {
                this->fireTargetEvents();

                TargetControllerComponent::notifier.waitForNotification(std::chrono::milliseconds(60));

                this->processQueuedCommands();
                this->eventListener->dispatchCurrentEvents();
            }

        } catch (const std::exception& exception) {
            Logger::error("The TargetController encountered a fatal error. See below for errors:");
            Logger::error(std::string(exception.what()));
        }

        this->shutdown();
    }

    void TargetControllerComponent::registerCommand(
        std::unique_ptr<Command> command,
        const std::optional<AtomicSessionIdType>& atomicSessionId
    ) {
        if (TargetControllerComponent::state != TargetControllerState::ACTIVE) {
            throw Exception("Command rejected - TargetController not in active state.");
        }

        if (atomicSessionId.has_value()) {
            // This command is part of an atomic session - put it in the dedicated queue
            TargetControllerComponent::atomicSessionCommandQueue.accessor()->push(std::move(command));
            TargetControllerComponent::notifier.notify();
            return;
        }

        TargetControllerComponent::commandQueue.accessor()->push(std::move(command));
        TargetControllerComponent::notifier.notify();
    }

    std::optional<std::unique_ptr<Responses::Response>> TargetControllerComponent::waitForResponse(
        CommandIdType commandId,
        std::optional<std::chrono::milliseconds> timeout
    ) {
        auto response = std::unique_ptr<Response>(nullptr);

        const auto predicate = [commandId, &response] {
            // We will already hold the lock here, so we can use Synchronised::unsafeReference() here.
            auto& responsesByCommandId = TargetControllerComponent::responsesByCommandId.unsafeReference();
            auto responseIt = responsesByCommandId.find(commandId);

            if (responseIt != responsesByCommandId.end()) {
                response.swap(responseIt->second);
                responsesByCommandId.erase(responseIt);

                return true;
            }

            return false;
        };

        auto responsesByCommandIdLock = TargetControllerComponent::responsesByCommandId.lock();

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
        this->threadState = ThreadState::STARTING;
        this->blockAllSignals();
        this->eventListener->setInterruptEventNotifier(&TargetControllerComponent::notifier);
        EventManager::registerListener(this->eventListener);

        // Register command handlers
        this->registerCommandHandler<StartAtomicSession>(
            std::bind(&TargetControllerComponent::handleStartAtomicSession, this, std::placeholders::_1)
        );

        this->registerCommandHandler<EndAtomicSession>(
            std::bind(&TargetControllerComponent::handleEndAtomicSession, this, std::placeholders::_1)
        );

        this->registerCommandHandler<Shutdown>(
            std::bind(&TargetControllerComponent::handleShutdown, this, std::placeholders::_1)
        );

        this->registerCommandHandler<GetTargetDescriptor>(
            std::bind(&TargetControllerComponent::handleGetTargetDescriptor, this, std::placeholders::_1)
        );

        this->registerCommandHandler<GetTargetState>(
            std::bind(&TargetControllerComponent::handleGetTargetState, this, std::placeholders::_1)
        );

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

        this->registerCommandHandler<ReadTargetMemory>(
            std::bind(&TargetControllerComponent::handleReadTargetMemory, this, std::placeholders::_1)
        );

        this->registerCommandHandler<WriteTargetMemory>(
            std::bind(&TargetControllerComponent::handleWriteTargetMemory, this, std::placeholders::_1)
        );

        this->registerCommandHandler<EraseTargetMemory>(
            std::bind(&TargetControllerComponent::handleEraseTargetMemory, this, std::placeholders::_1)
        );

        this->registerCommandHandler<StepTargetExecution>(
            std::bind(&TargetControllerComponent::handleStepTargetExecution, this, std::placeholders::_1)
        );

        this->registerCommandHandler<SetBreakpoint>(
            std::bind(&TargetControllerComponent::handleSetBreakpoint, this, std::placeholders::_1)
        );

        this->registerCommandHandler<RemoveBreakpoint>(
            std::bind(&TargetControllerComponent::handleRemoveBreakpoint, this, std::placeholders::_1)
        );

        this->registerCommandHandler<SetTargetProgramCounter>(
            std::bind(&TargetControllerComponent::handleSetProgramCounter, this, std::placeholders::_1)
        );

        this->registerCommandHandler<GetTargetPinStates>(
            std::bind(&TargetControllerComponent::handleGetTargetPinStates, this, std::placeholders::_1)
        );

        this->registerCommandHandler<SetTargetPinState>(
            std::bind(&TargetControllerComponent::handleSetTargetPinState, this, std::placeholders::_1)
        );

        this->registerCommandHandler<GetTargetStackPointer>(
            std::bind(&TargetControllerComponent::handleGetTargetStackPointer, this, std::placeholders::_1)
        );

        this->registerCommandHandler<GetTargetProgramCounter>(
            std::bind(&TargetControllerComponent::handleGetTargetProgramCounter, this, std::placeholders::_1)
        );

        this->registerCommandHandler<EnableProgrammingMode>(
            std::bind(&TargetControllerComponent::handleEnableProgrammingMode, this, std::placeholders::_1)
        );

        this->registerCommandHandler<DisableProgrammingMode>(
            std::bind(&TargetControllerComponent::handleDisableProgrammingMode, this, std::placeholders::_1)
        );

        // Register event handlers
        this->eventListener->registerCallbackForEventType<Events::ShutdownTargetController>(
            std::bind(&TargetControllerComponent::onShutdownTargetControllerEvent, this, std::placeholders::_1)
        );

        this->eventListener->registerCallbackForEventType<Events::DebugSessionFinished>(
            std::bind(&TargetControllerComponent::onDebugSessionFinishedEvent, this, std::placeholders::_1)
        );

        this->acquireHardware();
        this->loadRegisterDescriptors();

        if (this->target->getState() != TargetState::RUNNING) {
            this->target->run();
            this->lastTargetState = TargetState::RUNNING;
        }

        this->state = TargetControllerState::ACTIVE;
    }

    void TargetControllerComponent::shutdown() {
        const auto threadState = this->getThreadState();
        if (threadState == ThreadState::SHUTDOWN_INITIATED || threadState == ThreadState::STOPPED) {
            return;
        }

        this->threadState = ThreadState::SHUTDOWN_INITIATED;

        try {
            Logger::info("Shutting down TargetController");
            this->state = TargetControllerState::INACTIVE;
            EventManager::deregisterListener(this->eventListener->getId());

            if (this->activeAtomicSession.has_value()) {
                // Reject any commands on the dedicated queue
                this->processQueuedCommands();
                this->endActiveAtomicSession();
            }

            // Reject any commands still waiting in the queue
            this->processQueuedCommands();

            this->releaseHardware();

        } catch (const std::exception& exception) {
            this->target.reset();
            this->debugTool.reset();
            Logger::error(
                "Failed to properly shut down TargetController. Error: " + std::string(exception.what())
            );
        }

        this->setThreadStateAndEmitEvent(ThreadState::STOPPED);
    }

    std::map<
        std::string,
        std::function<std::unique_ptr<DebugTool>()>
    > TargetControllerComponent::getSupportedDebugTools() {
        // The debug tool names in this mapping should always be lower-case.
        return std::map<std::string, std::function<std::unique_ptr<DebugTool>()>> {
            {
                "atmel-ice",
                [] {
                    return std::make_unique<DebugToolDrivers::Microchip::AtmelIce>();
                }
            },
            {
                "power-debugger",
                [] {
                    return std::make_unique<DebugToolDrivers::Microchip::PowerDebugger>();
                }
            },
            {
                "snap",
                [] {
                    return std::make_unique<DebugToolDrivers::Microchip::MplabSnap>();
                }
            },
            {
                "pickit-4",
                [] {
                    return std::make_unique<DebugToolDrivers::Microchip::MplabPickit4>();
                }
            },
            {
                "xplained-pro",
                [] {
                    return std::make_unique<DebugToolDrivers::Microchip::XplainedPro>();
                }
            },
            {
                "xplained-mini",
                [] {
                    return std::make_unique<DebugToolDrivers::Microchip::XplainedMini>();
                }
            },
            {
                "xplained-nano",
                [] {
                    return std::make_unique<DebugToolDrivers::Microchip::XplainedNano>();
                }
            },
            {
                "curiosity-nano",
                [] {
                    return std::make_unique<DebugToolDrivers::Microchip::CuriosityNano>();
                }
            },
            {
                "jtagice3",
                [] {
                    return std::make_unique<DebugToolDrivers::Microchip::JtagIce3>();
                }
            },
            {
                "wch-link-e",
                [] {
                    return std::make_unique<DebugToolDrivers::Wch::WchLinkE>();
                }
            },
        };
    }

    std::map<
        std::string,
        std::function<std::unique_ptr<Targets::Target>(const TargetConfig&)>
    > TargetControllerComponent::getSupportedTargets() {
        using Avr8TargetDescriptionFile = Targets::Microchip::Avr::Avr8Bit::TargetDescription::TargetDescriptionFile;

        auto mapping = std::map<std::string, std::function<std::unique_ptr<Targets::Target>(const TargetConfig&)>>();

        // Include all targets from AVR8 target description files
        const auto avr8PdMapping = Avr8TargetDescriptionFile::getTargetDescriptionMapping();

        for (auto mapIt = avr8PdMapping.begin(); mapIt != avr8PdMapping.end(); ++mapIt) {
            const auto mappingObject = mapIt.value().toObject();
            const auto targetName = mappingObject.find("name").value().toString().toLower().toStdString();

            if (!mapping.contains(targetName)) {
                mapping.insert({
                    targetName,
                    [targetName] (const TargetConfig& targetConfig) {
                        return std::make_unique<Targets::Microchip::Avr::Avr8Bit::Avr8>(targetConfig);
                    }
                });
            }
        }

        return mapping;
    }

    void TargetControllerComponent::processQueuedCommands() {
        auto commands = std::queue<std::unique_ptr<Command>>();

        commands.swap(
            this->activeAtomicSession.has_value()
                ? *(TargetControllerComponent::atomicSessionCommandQueue.accessor())
                : *(TargetControllerComponent::commandQueue.accessor())
        );

        while (!commands.empty()) {
            const auto command = std::move(commands.front());
            commands.pop();

            const auto commandId = command->id;
            const auto commandType = command->getType();

            try {
                const auto commandHandlerIt = this->commandHandlersByCommandType.find(commandType);

                if (commandHandlerIt == this->commandHandlersByCommandType.end()) {
                    throw Exception("No handler registered for this command.");
                }

                if (this->state != TargetControllerState::ACTIVE) {
                    throw Exception("Command rejected - TargetController not in active state.");
                }

                if (command->requiresStoppedTargetState() && this->lastTargetState != TargetState::STOPPED) {
                    throw Exception("Command rejected - command requires target execution to be stopped.");
                }

                if (this->target->programmingModeEnabled() && command->requiresDebugMode()) {
                    throw Exception(
                        "Command rejected - command cannot be serviced whilst the target is in programming mode."
                    );
                }

                this->registerCommandResponse(commandId, commandHandlerIt->second(*(command.get())));

            } catch (const DeviceFailure& exception) {
                this->registerCommandResponse(
                    commandId,
                    std::make_unique<Responses::Error>(exception.getMessage())
                );

                throw exception;

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
        TargetControllerComponent::responsesByCommandId.accessor()->emplace(commandId, std::move(response));
        TargetControllerComponent::responsesByCommandIdCv.notify_all();
    }

    void TargetControllerComponent::acquireHardware() {
        const auto& debugToolName = this->environmentConfig.debugToolConfig.name;
        const auto& targetName = this->environmentConfig.targetConfig.name;

        static const auto supportedDebugTools = this->getSupportedDebugTools();
        static const auto supportedTargets = this->getSupportedTargets();

        const auto debugToolIt = supportedDebugTools.find(debugToolName);
        const auto targetIt = supportedTargets.find(targetName);

        if (debugToolIt == supportedDebugTools.end()) {
            throw Exceptions::InvalidConfig(
                "Debug tool name (\"" + debugToolName + "\") not recognised. Please check your configuration!"
            );
        }

        if (targetIt == supportedTargets.end()) {
            throw Exceptions::InvalidConfig(
                "Target name (\"" + targetName + "\") not recognised. Please check your configuration!"
            );
        }

        this->debugTool = debugToolIt->second();

        Logger::info("Connecting to debug tool");
        this->debugTool->init();

        Logger::info("Debug tool connected");
        Logger::info("Debug tool name: " + this->debugTool->getName());
        Logger::info("Debug tool serial: " + this->debugTool->getSerialNumber());
        Logger::info("Debug tool firmware version: " + this->debugTool->getFirmwareVersionString());

        this->target = targetIt->second(this->environmentConfig.targetConfig);
        const auto& targetDescriptor = this->getTargetDescriptor();

        if (!this->target->supportsDebugTool(this->debugTool.get())) {
            throw Exceptions::InvalidConfig(
                "Debug tool \"" + this->debugTool->getName() + "\" is not compatible with target \""
                    + targetDescriptor.name + "\"."
            );
        }

        this->target->setDebugTool(this->debugTool.get());

        Logger::info("Activating target");
        this->target->activate();
        Logger::info("Target activated");

        Logger::info("Target ID: " + targetDescriptor.id);
        Logger::info("Target name: " + targetDescriptor.name);

        if (!this->environmentConfig.targetConfig.hardwareBreakpoints) {
            Logger::warning("Hardware breakpoints have been disabled");

        } else {
            const auto& breakpointResources = targetDescriptor.breakpointResources;
            if (breakpointResources.maximumHardwareBreakpoints.has_value()) {
                Logger::info(
                    "Available hardware breakpoints: " + std::to_string(
                        *(breakpointResources.maximumHardwareBreakpoints)
                    )
                );
            }

            if (breakpointResources.reservedHardwareBreakpoints > 0) {
                Logger::info(
                    "Reserved hardware breakpoints: " + std::to_string(
                        breakpointResources.reservedHardwareBreakpoints
                    )
                );
            }
        }

        this->programMemoryCache = std::make_unique<Targets::TargetMemoryCache>(
            targetDescriptor.memoryDescriptorsByType.at(targetDescriptor.programMemoryType)
        );
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

    void TargetControllerComponent::startAtomicSession() {
        if (this->activeAtomicSession.has_value()) {
            throw Exception("Atomic session already active - nested sessions are not supported");
        }

        this->activeAtomicSession.emplace();
    }

    void TargetControllerComponent::endActiveAtomicSession() {
        if (!this->activeAtomicSession.has_value()) {
            return;
        }

        {
            auto commandQueue = TargetControllerComponent::atomicSessionCommandQueue.accessor();
            auto empty = std::queue<std::unique_ptr<Commands::Command>>();
            commandQueue->swap(empty);
        }

        this->activeAtomicSession.reset();
        TargetControllerComponent::notifier.notify();
    }

    void TargetControllerComponent::loadRegisterDescriptors() {
        const auto& targetDescriptor = this->getTargetDescriptor();

        for (const auto& [registerDescriptorId, registerDescriptor] : targetDescriptor.registerDescriptorsById) {
            auto startAddress = registerDescriptor.startAddress.value_or(0);
            auto endAddress = startAddress + (registerDescriptor.size - 1);

            const auto registerAddressRangeIt = this->registerAddressRangeByMemoryType.find(
                registerDescriptor.memoryType
            );

            if (registerAddressRangeIt == this->registerAddressRangeByMemoryType.end()) {
                this->registerAddressRangeByMemoryType.insert(
                    std::pair(registerDescriptor.memoryType, TargetMemoryAddressRange(startAddress, endAddress))
                );

            } else {
                auto& addressRange = registerAddressRangeIt->second;

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

    TargetRegisterDescriptors TargetControllerComponent::getRegisterDescriptorsWithinAddressRange(
        Targets::TargetMemoryAddress startAddress,
        Targets::TargetMemoryAddress endAddress,
        Targets::TargetMemoryType memoryType
    ) {
        auto output = TargetRegisterDescriptors();

        const auto registerAddressRangeIt = this->registerAddressRangeByMemoryType.find(memoryType);
        const auto registerDescriptorsIt = this->registerDescriptorsByMemoryType.find(memoryType);

        if (
            registerAddressRangeIt != this->registerAddressRangeByMemoryType.end()
            && registerDescriptorsIt != this->registerDescriptorsByMemoryType.end()
        ) {
            const auto& registersAddressRange = registerAddressRangeIt->second;

            if (
                (startAddress <= registersAddressRange.startAddress && endAddress >= registersAddressRange.startAddress)
                || (startAddress <= registersAddressRange.endAddress && endAddress >= registersAddressRange.startAddress)
            ) {
                const auto& registerDescriptors = registerDescriptorsIt->second;

                for (const auto& registerDescriptor : registerDescriptors) {
                    if (!registerDescriptor.startAddress.has_value() || registerDescriptor.size < 1) {
                        continue;
                    }

                    const auto registerStartAddress = registerDescriptor.startAddress.value();
                    const auto registerEndAddress = registerStartAddress + registerDescriptor.size;

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
                EventManager::triggerEvent(std::make_shared<TargetExecutionResumed>(false));
            }
        }
    }

    void TargetControllerComponent::resetTarget() {
        this->target->reset();

        EventManager::triggerEvent(std::make_shared<Events::TargetReset>());
    }

    void TargetControllerComponent::enableProgrammingMode() {
        Logger::debug("Enabling programming mode");
        this->target->enableProgrammingMode();
        Logger::warning("Programming mode enabled");

        EventManager::triggerEvent(std::make_shared<Events::ProgrammingModeEnabled>());
    }

    void TargetControllerComponent::disableProgrammingMode() {
        Logger::debug("Disabling programming mode");
        this->target->disableProgrammingMode();
        Logger::info("Programming mode disabled");

        EventManager::triggerEvent(std::make_shared<Events::ProgrammingModeDisabled>());

        Logger::info("Restoring breakpoints");
        this->target->stop();

        for (const auto& [address, breakpoint] : this->softwareBreakpointsByAddress) {
            this->target->setSoftwareBreakpoint(address);
        }

        for (const auto& [address, breakpoint] : this->hardwareBreakpointsByAddress) {
            this->target->setHardwareBreakpoint(address);
        }
    }

    const Targets::TargetDescriptor& TargetControllerComponent::getTargetDescriptor() {
        if (!this->targetDescriptor.has_value()) {
            this->targetDescriptor.emplace(this->target->getDescriptor());
        }

        return *this->targetDescriptor;
    }

    void TargetControllerComponent::onShutdownTargetControllerEvent(const Events::ShutdownTargetController&) {
        this->shutdown();
    }

    void TargetControllerComponent::onDebugSessionFinishedEvent(const DebugSessionFinished&) {
        if (this->state != TargetControllerState::ACTIVE) {
            return;
        }

        if (this->target->getState() != TargetState::RUNNING) {
            this->target->run();
            this->fireTargetEvents();
        }
    }

    std::unique_ptr<AtomicSessionId> TargetControllerComponent::handleStartAtomicSession(StartAtomicSession& command) {
        this->startAtomicSession();
        return std::make_unique<AtomicSessionId>(this->activeAtomicSession->id);
    }

    std::unique_ptr<Response> TargetControllerComponent::handleEndAtomicSession(EndAtomicSession& command) {
        if (!this->activeAtomicSession.has_value() || this->activeAtomicSession->id != command.sessionId) {
            throw Exception("Atomic session is not active");
        }

        this->endActiveAtomicSession();
        return std::make_unique<Response>();
    }

    std::unique_ptr<Response> TargetControllerComponent::handleShutdown(Shutdown& command) {
        this->shutdown();
        return std::make_unique<Response>();
    }

    std::unique_ptr<Responses::TargetDescriptor> TargetControllerComponent::handleGetTargetDescriptor(
        GetTargetDescriptor& command
    ) {
        return std::make_unique<Responses::TargetDescriptor>(this->getTargetDescriptor());
    }

    std::unique_ptr<Responses::TargetState> TargetControllerComponent::handleGetTargetState(GetTargetState& command) {
        return std::make_unique<Responses::TargetState>(this->target->getState());
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
            if (command.fromAddress.has_value()) {
                this->target->setProgramCounter(*command.fromAddress);
            }

            this->target->run(command.toAddress);
            this->lastTargetState = TargetState::RUNNING;
        }

        EventManager::triggerEvent(std::make_shared<Events::TargetExecutionResumed>(false));

        return std::make_unique<Response>();
    }

    std::unique_ptr<Response> TargetControllerComponent::handleResetTarget(ResetTarget& command) {
        this->resetTarget();
        return std::make_unique<Response>();
    }

    std::unique_ptr<TargetRegistersRead> TargetControllerComponent::handleReadTargetRegisters(
        ReadTargetRegisters& command
    ) {
        return std::make_unique<TargetRegistersRead>(
            !command.descriptorIds.empty()
                ? this->target->readRegisters(command.descriptorIds)
                : Targets::TargetRegisters()
        );
    }

    std::unique_ptr<Response> TargetControllerComponent::handleWriteTargetRegisters(WriteTargetRegisters& command) {
        if (!command.registers.empty()) {
            this->target->writeRegisters(command.registers);
        }

        auto registersWrittenEvent = std::make_shared<Events::RegistersWrittenToTarget>();
        registersWrittenEvent->registers = std::move(command.registers);

        EventManager::triggerEvent(registersWrittenEvent);

        return std::make_unique<Response>();
    }

    std::unique_ptr<TargetMemoryRead> TargetControllerComponent::handleReadTargetMemory(ReadTargetMemory& command) {
        const auto& targetDescriptor = this->getTargetDescriptor();
        if (
            command.memoryType == targetDescriptor.programMemoryType
            && !command.bypassCache
            && this->environmentConfig.targetConfig.programMemoryCache
        ) {
            assert(this->programMemoryCache);


            if (!this->programMemoryCache->contains(command.startAddress, command.bytes)) {
                Logger::debug(
                    "Program memory cache miss at 0x" + Services::StringService::toHex(command.startAddress) + ", "
                        + std::to_string(command.bytes) + " bytes"
                );

                /*
                 * TODO: We're currently ignoring command.excludedAddressRanges when populating the program
                 *       memory cache. This isn't a big deal, so I'll sort it later.
                 */
                this->programMemoryCache->insert(
                    command.startAddress,
                    this->target->readMemory(
                        command.memoryType,
                        command.startAddress,
                        std::max(
                            command.bytes,
                            targetDescriptor.memoryDescriptorsByType.at(command.memoryType).pageSize.value_or(0)
                        )
                    )
                );
            }

            return std::make_unique<TargetMemoryRead>(
                this->programMemoryCache->fetch(command.startAddress, command.bytes)
            );
        }

        return std::make_unique<TargetMemoryRead>(
            command.bytes > 0
                ? this->target->readMemory(
                    command.memoryType,
                    command.startAddress,
                    command.bytes,
                    command.excludedAddressRanges
                )
                : Targets::TargetMemoryBuffer()
        );
    }

    std::unique_ptr<Response> TargetControllerComponent::handleWriteTargetMemory(WriteTargetMemory& command) {
        const auto& buffer = command.buffer;
        const auto bufferSize = command.buffer.size();
        const auto bufferStartAddress = command.startAddress;

        const auto& targetDescriptor = this->getTargetDescriptor();

        if (command.memoryType == targetDescriptor.programMemoryType && !this->target->programmingModeEnabled()) {
            throw Exception("Cannot write to program memory - programming mode not enabled.");
        }

        this->target->writeMemory(command.memoryType, bufferStartAddress, buffer);

        if (
            command.memoryType == targetDescriptor.programMemoryType
            && this->environmentConfig.targetConfig.programMemoryCache
        ) {
            this->programMemoryCache->insert(bufferStartAddress, buffer);
        }

        EventManager::triggerEvent(
            std::make_shared<Events::MemoryWrittenToTarget>(command.memoryType, bufferStartAddress, bufferSize)
        );

        if (
            EventManager::isEventTypeListenedFor(Events::RegistersWrittenToTarget::type)
            && command.memoryType == targetDescriptor.programMemoryType
            && this->registerDescriptorsByMemoryType.contains(command.memoryType)
        ) {
            /*
             * The memory type we just wrote to contains some number of registers - if we've written to any address
             * that is known to store the value of a register, trigger a RegistersWrittenToTarget event
             */
            const auto bufferEndAddress = static_cast<std::uint32_t>(bufferStartAddress + (bufferSize - 1));
            auto registerDescriptors = this->getRegisterDescriptorsWithinAddressRange(
                bufferStartAddress,
                bufferEndAddress,
                command.memoryType
            );

            if (!registerDescriptors.empty()) {
                auto registersWrittenEvent = std::make_shared<Events::RegistersWrittenToTarget>();

                for (const auto& registerDescriptor : registerDescriptors) {
                    const auto registerSize = registerDescriptor.size;
                    const auto registerStartAddress = registerDescriptor.startAddress.value();
                    const auto registerEndAddress = registerStartAddress + (registerSize - 1);

                    if (registerStartAddress < bufferStartAddress || registerEndAddress > bufferEndAddress) {
                        continue;
                    }

                    const auto bufferBeginIt = buffer.begin() + (registerStartAddress - bufferStartAddress);
                    registersWrittenEvent->registers.emplace_back(TargetRegister(
                        registerDescriptor.id,
                        TargetMemoryBuffer(bufferBeginIt, bufferBeginIt + registerSize)
                    ));
                }

                EventManager::triggerEvent(registersWrittenEvent);
            }
        }

        return std::make_unique<Response>();
    }

    std::unique_ptr<Response> TargetControllerComponent::handleEraseTargetMemory(EraseTargetMemory& command) {
        if (command.memoryType == this->getTargetDescriptor().programMemoryType) {
            if (!this->target->programmingModeEnabled()) {
                throw Exception("Cannot erase program memory - programming mode not enabled.");
            }

            if (this->environmentConfig.targetConfig.programMemoryCache) {
                assert(this->programMemoryCache);

                Logger::debug("Clearing program memory cache");
                this->programMemoryCache->clear();
            }
        }

        this->target->eraseMemory(command.memoryType);

        return std::make_unique<Response>();
    }

    std::unique_ptr<Response> TargetControllerComponent::handleStepTargetExecution(StepTargetExecution& command) {
        if (command.fromProgramCounter.has_value()) {
            this->target->setProgramCounter(command.fromProgramCounter.value());
        }

        this->target->step();
        this->lastTargetState = TargetState::RUNNING;
        EventManager::triggerEvent(std::make_shared<Events::TargetExecutionResumed>(true));

        return std::make_unique<Response>();
    }

    std::unique_ptr<Breakpoint> TargetControllerComponent::handleSetBreakpoint(SetBreakpoint& command) {
        using Targets::TargetBreakpoint;
        using Services::StringService;

        auto breakpoint = TargetBreakpoint(command.address, TargetBreakpoint::Type::SOFTWARE);

        const auto& targetBreakpointResources = this->getTargetDescriptor().breakpointResources;
        if (
            command.preferredType == Targets::TargetBreakpoint::Type::HARDWARE
            && this->environmentConfig.targetConfig.hardwareBreakpoints
        ) {
            static auto exhaustedResourcesWarning = false;

            if (
                !targetBreakpointResources.maximumHardwareBreakpoints.has_value()
                || this->hardwareBreakpointsByAddress.size() < (*(targetBreakpointResources.maximumHardwareBreakpoints)
                    - targetBreakpointResources.reservedHardwareBreakpoints)
            ) {
                exhaustedResourcesWarning = true;

                Logger::debug(
                    "Installing hardware breakpoint at byte address 0x" + StringService::toHex(command.address)
                );

                this->target->setHardwareBreakpoint(command.address);
                this->hardwareBreakpointsByAddress.insert(std::pair(command.address, breakpoint));

                breakpoint.type = TargetBreakpoint::Type::HARDWARE;
                return std::make_unique<Breakpoint>(breakpoint);
            }

            if (exhaustedResourcesWarning) {
                exhaustedResourcesWarning = false;
                Logger::warning(
                    "Hardware breakpoint resources have been exhausted. Falling back to software breakpoints"
                );
            }
        }

        Logger::debug(
            "Installing software breakpoint at byte address 0x" + StringService::toHex(command.address)
        );

        this->target->setSoftwareBreakpoint(command.address);
        this->softwareBreakpointsByAddress.insert(std::pair(command.address, breakpoint));

        return std::make_unique<Breakpoint>(breakpoint);
    }

    std::unique_ptr<Response> TargetControllerComponent::handleRemoveBreakpoint(RemoveBreakpoint& command) {
        using Services::StringService;

        if (command.breakpoint.type == Targets::TargetBreakpoint::Type::HARDWARE) {
            assert(this->environmentConfig.targetConfig.hardwareBreakpoints);

            Logger::debug(
                "Removing hardware breakpoint at byte address 0x" + StringService::toHex(command.breakpoint.address)
            );

            this->target->removeHardwareBreakpoint(command.breakpoint.address);
            this->hardwareBreakpointsByAddress.erase(command.breakpoint.address);

        } else {
            Logger::debug(
                "Removing software breakpoint at byte address 0x" + StringService::toHex(command.breakpoint.address)
            );

            this->target->removeSoftwareBreakpoint(command.breakpoint.address);
            this->softwareBreakpointsByAddress.erase(command.breakpoint.address);
        }

        return std::make_unique<Response>();
    }

    std::unique_ptr<Response> TargetControllerComponent::handleSetProgramCounter(SetTargetProgramCounter& command) {
        this->target->setProgramCounter(command.address);
        return std::make_unique<Response>();
    }

    std::unique_ptr<TargetPinStates> TargetControllerComponent::handleGetTargetPinStates(GetTargetPinStates& command) {
        return std::make_unique<TargetPinStates>(this->target->getPinStates(command.variantId));
    }

    std::unique_ptr<Response> TargetControllerComponent::handleSetTargetPinState(SetTargetPinState& command) {
        this->target->setPinState(command.pinDescriptor, command.pinState);
        return std::make_unique<Response>();
    }

    std::unique_ptr<TargetStackPointer> TargetControllerComponent::handleGetTargetStackPointer(
        GetTargetStackPointer& command
    ) {
        return std::make_unique<TargetStackPointer>(this->target->getStackPointer());
    }

    std::unique_ptr<TargetProgramCounter> TargetControllerComponent::handleGetTargetProgramCounter(
        GetTargetProgramCounter& command
    ) {
        return std::make_unique<TargetProgramCounter>(this->target->getProgramCounter());
    }

    std::unique_ptr<Response> TargetControllerComponent::handleEnableProgrammingMode(EnableProgrammingMode& command) {
        if (!this->target->programmingModeEnabled()) {
            this->enableProgrammingMode();
        }

        return std::make_unique<Response>();
    }

    std::unique_ptr<Response> TargetControllerComponent::handleDisableProgrammingMode(DisableProgrammingMode& command) {
        if (this->target->programmingModeEnabled()) {
            this->disableProgrammingMode();
        }

        return std::make_unique<Response>();
    }
}
