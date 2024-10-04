#include "TargetControllerComponent.hpp"

#include <filesystem>
#include <typeindex>
#include <algorithm>

#include "src/Targets/Microchip/AVR8/TargetDescriptionFile.hpp"
#include "src/Targets/RiscV/TargetDescriptionFile.hpp"

#include "Responses/Error.hpp"

#include "src/Services/TargetService.hpp"
#include "src/Services/PathService.hpp"
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
    using Commands::SetTargetStackPointer;
    using Commands::GetTargetGpioPadStates;
    using Commands::SetTargetGpioPadState;
    using Commands::GetTargetStackPointer;
    using Commands::GetTargetProgramCounter;
    using Commands::EnableProgrammingMode;
    using Commands::DisableProgrammingMode;

    using Responses::Response;
    using Responses::AtomicSessionId;
    using Responses::TargetRegistersRead;
    using Responses::TargetMemoryRead;
    using Responses::TargetGpioPadStates;
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
                this->refreshExecutionState();

                TargetControllerComponent::notifier.waitForNotification(std::chrono::milliseconds(60));

                this->processQueuedCommands();
                this->eventListener->dispatchCurrentEvents();
            }

        } catch (const std::exception& exception) {
            Logger::error("The TargetController encountered a fatal error. See below for errors:");
            Logger::error(std::string{exception.what()});
        }

        this->shutdown();
    }

    void TargetControllerComponent::registerCommand(
        std::unique_ptr<Command> command,
        const std::optional<AtomicSessionIdType>& atomicSessionId
    ) {
        if (TargetControllerComponent::state != TargetControllerState::ACTIVE) {
            throw Exception{"Command rejected - TargetController not in active state."};
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

        return (response != nullptr) ? std::optional{std::move(response)} : std::nullopt;
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

        this->registerCommandHandler<SetTargetStackPointer>(
            std::bind(&TargetControllerComponent::handleSetStackPointer, this, std::placeholders::_1)
        );

        this->registerCommandHandler<SetTargetProgramCounter>(
            std::bind(&TargetControllerComponent::handleSetProgramCounter, this, std::placeholders::_1)
        );

        this->registerCommandHandler<GetTargetGpioPadStates>(
            std::bind(&TargetControllerComponent::handleGetTargetGpioPadStates, this, std::placeholders::_1)
        );

        this->registerCommandHandler<SetTargetGpioPadState>(
            std::bind(&TargetControllerComponent::handleSetTargetGpioPadState, this, std::placeholders::_1)
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

        if (this->targetState->executionState != TargetExecutionState::RUNNING) {
//            this->target->run();
//            this->targetState->executionState = TargetExecutionState::RUNNING;
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
                "Failed to properly shut down TargetController. Error: " + std::string{exception.what()}
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

    std::unique_ptr<Target> TargetControllerComponent::constructTarget(const BriefTargetDescriptor& briefDescriptor) {
        using Services::PathService;

        if (briefDescriptor.family == TargetFamily::AVR_8) {
            return std::make_unique<Microchip::Avr8::Avr8>(
                this->environmentConfig.targetConfig,
                Microchip::Avr8::TargetDescriptionFile{
                    PathService::targetDescriptionFilesDirPath() + "/" + briefDescriptor.relativeTdfPath
                }
            );
        }

        if (briefDescriptor.family == TargetFamily::RISC_V) {
            return std::make_unique<RiscV::RiscV>(
                this->environmentConfig.targetConfig,
                RiscV::TargetDescriptionFile{
                    PathService::targetDescriptionFilesDirPath() + "/" + briefDescriptor.relativeTdfPath
                }
            );
        }

        throw Exception{"Cannot construct target instance - invalid target family in BriefTargetDescriptor"};
    }

    void TargetControllerComponent::processQueuedCommands() {
        auto commands = std::queue<std::unique_ptr<Command>>{};

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
                    throw Exception{"No handler registered for this command."};
                }

                if (this->state != TargetControllerState::ACTIVE) {
                    throw Exception{"Command rejected - TargetController not in active state."};
                }

                if (
                    command->requiresStoppedTargetState()
                    && this->targetState->executionState != TargetExecutionState::STOPPED
                ) {
                    throw Exception{"Command rejected - command requires target execution to be stopped."};
                }

                if (this->target->programmingModeEnabled() && command->requiresDebugMode()) {
                    throw Exception{
                        "Command rejected - command cannot be serviced whilst the target is in programming mode."
                    };
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

        const auto debugToolIt = supportedDebugTools.find(debugToolName);
        const auto briefTargetDescriptor = Services::TargetService::briefDescriptor(targetName);

        if (debugToolIt == supportedDebugTools.end()) {
            throw Exceptions::InvalidConfig{
                "Debug tool name (\"" + debugToolName + "\") not recognised. Please check your configuration!"
            };
        }

        if (!briefTargetDescriptor.has_value()) {
            throw Exceptions::InvalidConfig{
                "Target name (\"" + targetName + "\") not recognised. Please check your configuration!"
            };
        }

        this->debugTool = debugToolIt->second();

        Logger::info("Connecting to debug tool");
        this->debugTool->init();
        Logger::info("Debug tool connected");

        this->debugTool->postInit();

        Logger::info("Debug tool name: " + this->debugTool->getName());
        Logger::info("Debug tool serial: " + this->debugTool->getSerialNumber());

        this->target = this->constructTarget(*briefTargetDescriptor);

        if (!this->target->supportsDebugTool(this->debugTool.get())) {
            throw Exceptions::InvalidConfig{
                "Debug tool \"" + this->debugTool->getName() + "\" is not compatible with target \""
                    + targetName + "\"."
            };
        }

        this->target->setDebugTool(this->debugTool.get());

        Logger::info("Activating target");
        this->target->activate();
        Logger::info("Target activated");

        this->targetDescriptor = std::make_unique<const TargetDescriptor>(this->target->targetDescriptor());
        Logger::info("Target name: " + this->targetDescriptor->name);

        this->target->postActivate();

        this->targetState = std::make_unique<TargetState>(
            TargetExecutionState::UNKNOWN,
            TargetMode::DEBUGGING,
            std::nullopt
        );
        this->refreshExecutionState();

        if (!this->environmentConfig.targetConfig.hardwareBreakpoints) {
            Logger::warning("Hardware breakpoints have been disabled");

        } else {
            const auto& breakpointResources = this->targetDescriptor->breakpointResources;
            if (breakpointResources.maximumHardwareBreakpoints.has_value()) {
                Logger::info(
                    "Available hardware breakpoints: " + std::to_string(
                        *(breakpointResources.maximumHardwareBreakpoints)
                    )
                );
            }

            if (breakpointResources.reservedHardwareBreakpoints > 0) {
                Logger::info(
                    "Reserved hardware breakpoints: " + std::to_string(breakpointResources.reservedHardwareBreakpoints)
                );
            }
        }
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
            throw Exception{"Atomic session already active - nested sessions are not supported"};
        }

        this->activeAtomicSession.emplace();
    }

    void TargetControllerComponent::endActiveAtomicSession() {
        if (!this->activeAtomicSession.has_value()) {
            return;
        }

        {
            auto commandQueue = TargetControllerComponent::atomicSessionCommandQueue.accessor();
            auto empty = std::queue<std::unique_ptr<Commands::Command>>{};
            commandQueue->swap(empty);
        }

        this->activeAtomicSession.reset();
        TargetControllerComponent::notifier.notify();
    }

    void TargetControllerComponent::refreshExecutionState() {
        auto newExecutionState = this->target->getExecutionState();

        if (newExecutionState != this->targetState->executionState) {
            Logger::debug("Target execution state changed");

            auto newState = *(this->targetState);
            newState.executionState = newExecutionState;

            if (newExecutionState == TargetExecutionState::STOPPED) {
                Logger::debug("Target stopped");
                newState.programCounter = this->target->getProgramCounter();

            } else {
                Logger::debug("Target resumed");
                newState.programCounter = std::nullopt;
            }

            this->updateTargetState(newState);
        }
    }

    void TargetControllerComponent::updateTargetState(const TargetState& newState) {
        if (newState == *(this->targetState)) {
            // Nothing has changed, nothing to do
            return;
        }

        const auto previousState = *(this->targetState);
        *(this->targetState) = newState;
        EventManager::triggerEvent(std::make_shared<TargetStateChanged>(*(this->targetState), previousState));
    }

    void TargetControllerComponent::stopTarget() {
        if (this->target->getExecutionState() != TargetExecutionState::STOPPED) {
            this->target->stop();
        }

        auto newState = *(this->targetState);
        newState.executionState = TargetExecutionState::STOPPED;
        newState.programCounter = this->target->getProgramCounter();
        this->updateTargetState(newState);
    }

    void TargetControllerComponent::resumeTarget() {
        if (this->target->getExecutionState() != TargetExecutionState::RUNNING) {
            this->target->run(std::nullopt);
        }

        auto newState = *(this->targetState);
        newState.executionState = TargetExecutionState::RUNNING;
        newState.programCounter = std::nullopt;
        this->updateTargetState(newState);
    }

    void TargetControllerComponent::stepTarget() {
        this->target->step();

        auto newState = *(this->targetState);
        newState.executionState = TargetExecutionState::STEPPING;
        newState.programCounter = std::nullopt;
        this->updateTargetState(newState);
    }

    void TargetControllerComponent::resetTarget() {
        this->target->reset();
        EventManager::triggerEvent(std::make_shared<Events::TargetReset>());
    }

    TargetRegisterDescriptorAndValuePairs TargetControllerComponent::readTargetRegisters(
        const TargetRegisterDescriptors& descriptors
    ) {
        return this->target->readRegisters(descriptors);
    }

    void TargetControllerComponent::writeTargetRegisters(const TargetRegisterDescriptorAndValuePairs& registers) {
        this->target->writeRegisters(registers);
        EventManager::triggerEvent(std::make_shared<Events::RegistersWrittenToTarget>(registers));
    }

    Targets::TargetMemoryBuffer TargetControllerComponent::readTargetMemory(
        const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const TargetMemorySegmentDescriptor& memorySegmentDescriptor,
        TargetMemoryAddress startAddress,
        TargetMemorySize bytes,
        const std::set<TargetMemoryAddressRange>& excludedAddressRanges,
        bool bypassCache
    ) {
        if (
            !bypassCache
            && this->environmentConfig.targetConfig.programMemoryCache
            && this->target->isProgramMemory(addressSpaceDescriptor, memorySegmentDescriptor, startAddress, bytes)
        ) {
            auto& cache = this->getProgramMemoryCache(addressSpaceDescriptor);

            if (!cache.contains(startAddress, bytes)) {
                Logger::debug(
                    "Program memory cache miss at 0x" + Services::StringService::toHex(startAddress) + ", "
                        + std::to_string(bytes) + " bytes"
                );

                /*
                 * TODO: We're currently ignoring excludedAddressRanges when populating the program
                 *       memory cache. This isn't a big deal, so I'll sort it later.
                 */
                cache.insert(
                    startAddress,
                    this->target->readMemory(
                        addressSpaceDescriptor,
                        memorySegmentDescriptor,
                        startAddress,
                        std::max(
                            bytes,
                            memorySegmentDescriptor.pageSize.value_or(0)
                        ),
                        {}
                    )
                );
            }

            return cache.fetch(startAddress, bytes);
        }

        return this->target->readMemory(
            addressSpaceDescriptor,
            memorySegmentDescriptor,
            startAddress,
            bytes,
            excludedAddressRanges
        );
    }

    void TargetControllerComponent::writeTargetMemory(
        const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const TargetMemorySegmentDescriptor& memorySegmentDescriptor,
        Targets::TargetMemoryAddress startAddress,
        const TargetMemoryBuffer& buffer
    ) {
        const auto isProgramMemory = this->target->isProgramMemory(
            addressSpaceDescriptor,
            memorySegmentDescriptor,
            startAddress,
            static_cast<TargetMemorySize>(buffer.size())
        );

        if (isProgramMemory && !this->target->programmingModeEnabled()) {
            throw Exception{"Cannot write to program memory - programming mode not enabled."};
        }

        this->target->writeMemory(addressSpaceDescriptor, memorySegmentDescriptor, startAddress, buffer);

        if (isProgramMemory && this->environmentConfig.targetConfig.programMemoryCache) {
            this->getProgramMemoryCache(addressSpaceDescriptor).insert(startAddress, buffer);
        }

        EventManager::triggerEvent(
            std::make_shared<Events::MemoryWrittenToTarget>(
                addressSpaceDescriptor,
                memorySegmentDescriptor,
                startAddress,
                static_cast<TargetMemorySize>(buffer.size())
            )
        );
    }

    void TargetControllerComponent::eraseTargetMemory(
        const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const TargetMemorySegmentDescriptor& memorySegmentDescriptor
    ) {
        if (this->target->isProgramMemory(
            addressSpaceDescriptor,
            memorySegmentDescriptor,
            memorySegmentDescriptor.addressRange.startAddress,
            memorySegmentDescriptor.addressRange.size()
        )) {
            if (!this->target->programmingModeEnabled()) {
                throw Exception{"Cannot erase program memory - programming mode not enabled."};
            }

            if (this->environmentConfig.targetConfig.programMemoryCache) {
                Logger::debug("Clearing program memory cache");
                this->getProgramMemoryCache(addressSpaceDescriptor).clear();
            }
        }

        this->target->eraseMemory(addressSpaceDescriptor, memorySegmentDescriptor);
    }

    void TargetControllerComponent::setBreakpoint(const TargetBreakpoint& breakpoint) {
        using Services::StringService;

        if (breakpoint.type == TargetBreakpoint::Type::HARDWARE) {
            Logger::debug(
                "Installing hardware breakpoint at byte address 0x" + StringService::toHex(breakpoint.address)
            );

            this->target->setHardwareBreakpoint(breakpoint.address);
            this->hardwareBreakpointsByAddress.emplace(breakpoint.address, breakpoint);
            return;
        }

        Logger::debug("Installing software breakpoint at byte address 0x" + StringService::toHex(breakpoint.address));

        this->target->setSoftwareBreakpoint(breakpoint.address);
        this->softwareBreakpointsByAddress.emplace(breakpoint.address, breakpoint);
    }

    void TargetControllerComponent::removeBreakpoint(const TargetBreakpoint& breakpoint) {
        using Services::StringService;

        if (breakpoint.type == Targets::TargetBreakpoint::Type::HARDWARE) {
            Logger::debug("Removing hardware breakpoint at byte address 0x" + StringService::toHex(breakpoint.address));

            this->target->removeHardwareBreakpoint(breakpoint.address);
            this->hardwareBreakpointsByAddress.erase(breakpoint.address);
            return;
        }

        Logger::debug("Removing software breakpoint at byte address 0x" + StringService::toHex(breakpoint.address));

        this->target->removeSoftwareBreakpoint(breakpoint.address);
        this->softwareBreakpointsByAddress.erase(breakpoint.address);
    }

    void TargetControllerComponent::enableProgrammingMode() {
        Logger::debug("Enabling programming mode");
        this->target->enableProgrammingMode();
        Logger::warning("Programming mode enabled");

        auto newState = *(this->targetState);
        newState.mode = TargetMode::PROGRAMMING;
        this->updateTargetState(newState);
    }

    void TargetControllerComponent::disableProgrammingMode() {
        Logger::debug("Disabling programming mode");
        this->target->disableProgrammingMode();
        Logger::info("Programming mode disabled");

        Logger::info("Restoring breakpoints");
        this->target->stop();

        for (const auto& [address, breakpoint] : this->softwareBreakpointsByAddress) {
            this->target->setSoftwareBreakpoint(address);
        }

        for (const auto& [address, breakpoint] : this->hardwareBreakpointsByAddress) {
            this->target->setHardwareBreakpoint(address);
        }

        auto newState = *(this->targetState);
        newState.mode = TargetMode::DEBUGGING;
        newState.executionState = TargetExecutionState::STOPPED;
        this->updateTargetState(newState);
    }

    TargetMemoryCache& TargetControllerComponent::getProgramMemoryCache(
        const TargetAddressSpaceDescriptor& addressSpaceDescriptor
    ) {
        auto cacheIt = this->programMemoryCachesByAddressSpaceKey.find(addressSpaceDescriptor.key);

        if (cacheIt == this->programMemoryCachesByAddressSpaceKey.end()) {
            cacheIt = this->programMemoryCachesByAddressSpaceKey.emplace(
                addressSpaceDescriptor.key,
                TargetMemoryCache{addressSpaceDescriptor}
            ).first;
        }

        return cacheIt->second;
    }

    void TargetControllerComponent::onShutdownTargetControllerEvent(const Events::ShutdownTargetController&) {
        this->shutdown();
    }

    void TargetControllerComponent::onDebugSessionFinishedEvent(const DebugSessionFinished&) {
        if (this->state != TargetControllerState::ACTIVE) {
            return;
        }

        if (this->target->getExecutionState() != TargetExecutionState::RUNNING) {
            this->target->run(std::nullopt);
            this->refreshExecutionState();
        }
    }

    std::unique_ptr<AtomicSessionId> TargetControllerComponent::handleStartAtomicSession(StartAtomicSession& command) {
        this->startAtomicSession();
        return std::make_unique<AtomicSessionId>(this->activeAtomicSession->id);
    }

    std::unique_ptr<Response> TargetControllerComponent::handleEndAtomicSession(EndAtomicSession& command) {
        if (!this->activeAtomicSession.has_value() || this->activeAtomicSession->id != command.sessionId) {
            throw Exception{"Atomic session is not active"};
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
        return std::make_unique<Responses::TargetDescriptor>(*(this->targetDescriptor));
    }

    std::unique_ptr<Responses::TargetState> TargetControllerComponent::handleGetTargetState(GetTargetState& command) {
        return std::make_unique<Responses::TargetState>(*(this->targetState));
    }

    std::unique_ptr<Response> TargetControllerComponent::handleStopTargetExecution(StopTargetExecution& command) {
        this->stopTarget();
        return std::make_unique<Response>();
    }

    std::unique_ptr<Response> TargetControllerComponent::handleResumeTargetExecution(ResumeTargetExecution& command) {
        this->resumeTarget();
        return std::make_unique<Response>();
    }

    std::unique_ptr<Response> TargetControllerComponent::handleResetTarget(ResetTarget& command) {
        this->resetTarget();
        return std::make_unique<Response>();
    }

    std::unique_ptr<TargetRegistersRead> TargetControllerComponent::handleReadTargetRegisters(
        ReadTargetRegisters& command
    ) {
        if (command.descriptors.empty()) {
            throw Exception{"No register descriptors provided"};
        }

        return std::make_unique<TargetRegistersRead>(this->readTargetRegisters(command.descriptors));
    }

    std::unique_ptr<Response> TargetControllerComponent::handleWriteTargetRegisters(WriteTargetRegisters& command) {
        if (command.registers.empty()) {
            throw Exception{"No register values provided"};
        }

        this->writeTargetRegisters(std::move(command.registers));
        return std::make_unique<Response>();
    }

    std::unique_ptr<TargetMemoryRead> TargetControllerComponent::handleReadTargetMemory(ReadTargetMemory& command) {
        if (command.bytes == 0) {
            throw Exception{"Zero bytes requested"};
        }

        const auto addressRange = TargetMemoryAddressRange(
            command.startAddress,
            command.startAddress + command.bytes - 1
        );

        if (
            !command.addressSpaceDescriptor.addressRange.contains(addressRange)
            || !command.memorySegmentDescriptor.addressRange.contains(addressRange)
        ) {
            throw Exception{"Invalid address range"};
        }

        return std::make_unique<TargetMemoryRead>(
            this->readTargetMemory(
                command.addressSpaceDescriptor,
                command.memorySegmentDescriptor,
                command.startAddress,
                command.bytes,
                command.excludedAddressRanges,
                command.bypassCache
            )
        );
    }

    std::unique_ptr<Response> TargetControllerComponent::handleWriteTargetMemory(WriteTargetMemory& command) {
        if (command.buffer.empty()) {
            throw Exception{"Empty buffer"};
        }

        const auto addressRange = TargetMemoryAddressRange{
            command.startAddress,
            static_cast<TargetMemoryAddress>(command.startAddress + command.buffer.size() - 1)
        };

        if (
            !command.addressSpaceDescriptor.addressRange.contains(addressRange)
            || !command.memorySegmentDescriptor.addressRange.contains(addressRange)
        ) {
            throw Exception{"Invalid address range"};
        }

        this->writeTargetMemory(
            command.addressSpaceDescriptor,
            command.memorySegmentDescriptor,
            command.startAddress,
            command.buffer
        );

        return std::make_unique<Response>();
    }

    std::unique_ptr<Response> TargetControllerComponent::handleEraseTargetMemory(EraseTargetMemory& command) {
        this->eraseTargetMemory(command.addressSpaceDescriptor, command.memorySegmentDescriptor);
        return std::make_unique<Response>();
    }

    std::unique_ptr<Response> TargetControllerComponent::handleStepTargetExecution(StepTargetExecution&) {
        this->stepTarget();
        return std::make_unique<Response>();
    }

    std::unique_ptr<Breakpoint> TargetControllerComponent::handleSetBreakpoint(SetBreakpoint& command) {
        using Targets::TargetBreakpoint;
        using Services::StringService;

        const auto& targetBreakpointResources = this->targetDescriptor->breakpointResources;
        if (
            command.preferredType == Targets::TargetBreakpoint::Type::HARDWARE
            && this->environmentConfig.targetConfig.hardwareBreakpoints
            && (
                !targetBreakpointResources.maximumHardwareBreakpoints.has_value()
                || this->hardwareBreakpointsByAddress.size() < (*(targetBreakpointResources.maximumHardwareBreakpoints)
                    - targetBreakpointResources.reservedHardwareBreakpoints)
            )
        ) {
            const auto hwBreakpoint = TargetBreakpoint{command.address, TargetBreakpoint::Type::HARDWARE};
            this->setBreakpoint(hwBreakpoint);

            return std::make_unique<Breakpoint>(hwBreakpoint);
        }

        const auto swBreakpoint = TargetBreakpoint(command.address, TargetBreakpoint::Type::SOFTWARE);
        this->setBreakpoint(swBreakpoint);

        return std::make_unique<Breakpoint>(swBreakpoint);
    }

    std::unique_ptr<Response> TargetControllerComponent::handleRemoveBreakpoint(RemoveBreakpoint& command) {
        this->removeBreakpoint(command.breakpoint);
        return std::make_unique<Response>();
    }

    std::unique_ptr<Response> TargetControllerComponent::handleSetProgramCounter(SetTargetProgramCounter& command) {
        this->target->setProgramCounter(command.address);
        return std::make_unique<Response>();
    }

    std::unique_ptr<Response> TargetControllerComponent::handleSetStackPointer(SetTargetStackPointer& command) {
        this->target->setStackPointer(command.stackPointer);
        return std::make_unique<Response>();
    }

    std::unique_ptr<TargetGpioPadStates> TargetControllerComponent::handleGetTargetGpioPadStates(
        GetTargetGpioPadStates& command
    ) {
        return std::make_unique<TargetGpioPadStates>(this->target->getGpioPadStates(command.padDescriptors));
    }

    std::unique_ptr<Response> TargetControllerComponent::handleSetTargetGpioPadState(SetTargetGpioPadState& command) {
        this->target->setGpioPadState(command.padDescriptor, command.state);
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

    std::unique_ptr<Response> TargetControllerComponent::handleEnableProgrammingMode(EnableProgrammingMode&) {
        if (!this->target->programmingModeEnabled()) {
            this->enableProgrammingMode();
        }

        return std::make_unique<Response>();
    }

    std::unique_ptr<Response> TargetControllerComponent::handleDisableProgrammingMode(DisableProgrammingMode&) {
        if (this->target->programmingModeEnabled()) {
            this->disableProgrammingMode();
        }

        return std::make_unique<Response>();
    }
}
