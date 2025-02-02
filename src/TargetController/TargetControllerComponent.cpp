#include "TargetControllerComponent.hpp"

#include <filesystem>
#include <typeindex>
#include <algorithm>

#include "src/Targets/Microchip/Avr8/TargetDescriptionFile.hpp"
#include "src/Targets/RiscV/Wch/TargetDescriptionFile.hpp"

#include "Responses/Error.hpp"

#include "src/Services/TargetService.hpp"
#include "src/Services/PathService.hpp"
#include "src/Services/ProcessService.hpp"
#include "src/Services/StringService.hpp"
#include "src/Services/AlignmentService.hpp"
#include "src/Logger/Logger.hpp"

#include "Exceptions/TargetOperationFailure.hpp"

#include "src/Exceptions/FatalErrorException.hpp"
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
    using Commands::SetProgramBreakpointAnyType;
    using Commands::RemoveProgramBreakpoint;
    using Commands::SetTargetProgramCounter;
    using Commands::SetTargetStackPointer;
    using Commands::GetTargetGpioPadStates;
    using Commands::SetTargetGpioPadState;
    using Commands::GetTargetStackPointer;
    using Commands::GetTargetProgramCounter;
    using Commands::EnableProgrammingMode;
    using Commands::DisableProgrammingMode;
    using Commands::GetTargetPassthroughHelpText;
    using Commands::InvokeTargetPassthroughCommand;

    using Responses::Response;
    using Responses::AtomicSessionId;
    using Responses::TargetRegistersRead;
    using Responses::TargetMemoryRead;
    using Responses::TargetGpioPadStates;
    using Responses::TargetStackPointer;
    using Responses::TargetProgramCounter;
    using Responses::ProgramBreakpoint;
    using Responses::TargetPassthroughHelpText;
    using Responses::TargetPassthroughResponse;

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

                TargetControllerComponent::notifier.waitForNotification(std::chrono::milliseconds{60});

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

        this->registerCommandHandler<SetProgramBreakpointAnyType>(
            std::bind(
                &TargetControllerComponent::handleSetProgramBreakpointBreakpointAnyType,
                this,
                std::placeholders::_1
            )
        );

        this->registerCommandHandler<RemoveProgramBreakpoint>(
            std::bind(&TargetControllerComponent::handleRemoveProgramBreakpoint, this, std::placeholders::_1)
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

        this->registerCommandHandler<GetTargetPassthroughHelpText>(
            std::bind(&TargetControllerComponent::handleTargetPassthroughHelpText, this, std::placeholders::_1)
        );

        this->registerCommandHandler<InvokeTargetPassthroughCommand>(
            std::bind(&TargetControllerComponent::handleTargetPassthroughCommand, this, std::placeholders::_1)
        );

        // Register event handlers
        this->eventListener->registerCallbackForEventType<Events::ShutdownTargetController>(
            std::bind(&TargetControllerComponent::onShutdownTargetControllerEvent, this, std::placeholders::_1)
        );

        this->acquireHardware();

        this->targetState = std::make_unique<TargetState>(
            TargetExecutionState::UNKNOWN,
            TargetMode::DEBUGGING,
            std::nullopt
        );
        this->refreshExecutionState();

        if (this->environmentConfig.targetConfig.hardwareBreakpoints) {
            const auto& breakpointResources = this->targetDescriptor->breakpointResources;
            Logger::info("Available hardware breakpoints: " + std::to_string(breakpointResources.hardwareBreakpoints));
            Logger::info(
                "Reserved hardware breakpoints: " + std::to_string(breakpointResources.reservedHardwareBreakpoints)
            );

        } else {
            Logger::warning("Hardware breakpoints have been disabled");
        }

        if (
            this->targetState->executionState == TargetExecutionState::STOPPED
            && this->environmentConfig.targetConfig.resumeOnStartup
        ) {
            try {
                this->resumeTarget();

            } catch (const Exceptions::TargetOperationFailure& exception) {
                Logger::error("Failed to resume target execution on startup - error: " + exception.getMessage());
            }
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

            if (this->target != nullptr && this->targetState != nullptr) {
                // Cleanup before deactivating the target
                try {
                    this->stopTarget();
                    this->clearAllBreakpoints();

                } catch (const Exception& exception) {
                    Logger::error("Target pre-deactivation cleanup failed: " + exception.getMessage());
                }
            }

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
        using namespace DebugToolDrivers::Microchip;
        using namespace DebugToolDrivers::Wch;

        // The debug tool names in this mapping should always be lower-case.
        return std::map<std::string, std::function<std::unique_ptr<DebugTool>()>> {
            {
                "atmel_ice",
                [this] {
                    return std::make_unique<AtmelIce>(this->environmentConfig.debugToolConfig);
                }
            },
            {
                "power_debugger",
                [this] {
                    return std::make_unique<PowerDebugger>(this->environmentConfig.debugToolConfig);
                }
            },
            {
                "snap",
                [this] {
                    return std::make_unique<MplabSnap>(this->environmentConfig.debugToolConfig);
                }
            },
            {
                "pickit_4",
                [this] {
                    return std::make_unique<MplabPickit4>(this->environmentConfig.debugToolConfig);
                }
            },
            {
                "xplained_pro",
                [this] {
                    return std::make_unique<XplainedPro>(this->environmentConfig.debugToolConfig);
                }
            },
            {
                "xplained_mini",
                [this] {
                    return std::make_unique<XplainedMini>(this->environmentConfig.debugToolConfig);
                }
            },
            {
                "xplained_nano",
                [this] {
                    return std::make_unique<XplainedNano>(this->environmentConfig.debugToolConfig);
                }
            },
            {
                "curiosity_nano",
                [this] {
                    return std::make_unique<CuriosityNano>(this->environmentConfig.debugToolConfig);
                }
            },
            {
                "jtagice3",
                [this] {
                    return std::make_unique<JtagIce3>(this->environmentConfig.debugToolConfig);
                }
            },
            {
                "wch_linke",
                [this] {
                    return std::make_unique<WchLinkE>(this->environmentConfig.debugToolConfig);
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
            /*
             * Given that WCH targets are the only RISC-V targets we support ATM, we can just assume that
             * construction of a WchRiscV object is necessary.
             *
             * TODO: Review later
             */
            return std::make_unique<RiscV::Wch::WchRiscV>(
                this->environmentConfig.targetConfig,
                RiscV::Wch::TargetDescriptionFile{
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

            } catch (const FatalErrorException& exception) {
                this->registerCommandResponse(
                    commandId,
                    std::make_unique<Responses::Error>(exception.getMessage())
                );

                throw exception;

            } catch (const Exception& exception) {
                try {
                    this->refreshExecutionState(true);

                } catch (const Exception& refreshException) {
                    Logger::error("Post exception target state refresh failed: " + refreshException.getMessage());
                }

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
                "Debug tool (\"" + debugToolName + "\") not recognised. Please check your configuration."
            };
        }

        if (!briefTargetDescriptor.has_value()) {
            throw Exceptions::InvalidConfig{
                "Target (\"" + targetName + "\") not recognised. Please check your configuration."
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

        this->deltaProgrammingInterface = this->target->deltaProgrammingInterface();
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

    void TargetControllerComponent::refreshExecutionState(bool forceUpdate) {
        auto newState = *(this->targetState);
        newState.executionState = this->target->getExecutionState();

        if (!forceUpdate && newState.executionState == this->targetState->executionState) {
            return;
        }

        newState.programCounter = newState.executionState == TargetExecutionState::STOPPED
            ? std::optional{this->target->getProgramCounter()}
            : std::nullopt;
        this->updateTargetState(newState);
    }

    void TargetControllerComponent::updateTargetState(const TargetState& newState) {
        const auto previousState = *(this->targetState);
        *(this->targetState) = newState;

        if (newState != previousState) {
            EventManager::triggerEvent(std::make_shared<TargetStateChanged>(*(this->targetState), previousState));
        }
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
            auto& cache = this->getProgramMemoryCache(memorySegmentDescriptor);

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

            const auto cachedData = cache.fetch(startAddress, bytes);
            return TargetMemoryBuffer{cachedData.begin(), cachedData.end()};
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
        TargetMemoryAddress startAddress,
        TargetMemoryBufferSpan buffer
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

        if (
            isProgramMemory
            && (
                this->environmentConfig.targetConfig.programMemoryCache
                || this->environmentConfig.targetConfig.deltaProgramming
            )
        ) {
            this->getProgramMemoryCache(memorySegmentDescriptor).insert(startAddress, buffer);
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

            if (
                this->environmentConfig.targetConfig.programMemoryCache
                || this->environmentConfig.targetConfig.deltaProgramming
            ) {
                Logger::debug("Clearing program memory cache");
                this->getProgramMemoryCache(memorySegmentDescriptor).clear();
            }
        }

        this->target->eraseMemory(addressSpaceDescriptor, memorySegmentDescriptor);
    }

    std::uint32_t TargetControllerComponent::availableHardwareBreakpoints() {
        const auto& targetBreakpointResources = this->targetDescriptor->breakpointResources;
        return static_cast<std::uint32_t>(
            targetBreakpointResources.hardwareBreakpoints - targetBreakpointResources.reservedHardwareBreakpoints
                - this->hardwareBreakpointRegistry.size()
        );
    }

    void TargetControllerComponent::setProgramBreakpoint(const TargetProgramBreakpoint& breakpoint) {
        using Services::StringService;

        auto& registry = breakpoint.type == TargetProgramBreakpoint::Type::HARDWARE
            ? this->hardwareBreakpointRegistry
            : this->softwareBreakpointRegistry;

        Logger::debug("Inserting breakpoint at byte address 0x" + StringService::toHex(breakpoint.address));

        if (registry.contains(breakpoint)) {
            Logger::debug("Breakpoint already in registry - ignoring insertion request");
            return;
        }

        this->target->setProgramBreakpoint(breakpoint);
        registry.insert(breakpoint);

        if (
            breakpoint.type == TargetProgramBreakpoint::Type::SOFTWARE
            && (
                this->environmentConfig.targetConfig.programMemoryCache
                || this->environmentConfig.targetConfig.deltaProgramming
            )
            && this->target->isProgramMemory(
                breakpoint.addressSpaceDescriptor,
                breakpoint.memorySegmentDescriptor,
                breakpoint.address,
                breakpoint.size
            )
        ) {
            auto& cache = this->getProgramMemoryCache(breakpoint.memorySegmentDescriptor);
            if (cache.contains(breakpoint.address, breakpoint.size)) {
                // Update program memory cache
                cache.insert(
                    breakpoint.address,
                    this->target->readMemory(
                        breakpoint.addressSpaceDescriptor,
                        breakpoint.memorySegmentDescriptor,
                        breakpoint.address,
                        breakpoint.size,
                        {}
                    )
                );
            }
        }
    }

    void TargetControllerComponent::removeProgramBreakpoint(const TargetProgramBreakpoint& breakpoint) {
        using Services::StringService;

        auto& registry = breakpoint.type == TargetProgramBreakpoint::Type::HARDWARE
            ? this->hardwareBreakpointRegistry
            : this->softwareBreakpointRegistry;

        Logger::debug("Removing breakpoint at byte address 0x" + StringService::toHex(breakpoint.address));


        const auto registeredBreakpointOpt = registry.find(breakpoint);
        if (!registeredBreakpointOpt.has_value()) {
            Logger::debug("Breakpoint not found in registry - ignoring removal request");
            return;
        }

        const auto& registeredBreakpoint = registeredBreakpointOpt->get();
        this->target->removeProgramBreakpoint(registeredBreakpoint);

        if (
            registeredBreakpoint.type == TargetProgramBreakpoint::Type::SOFTWARE
            && (
                this->environmentConfig.targetConfig.programMemoryCache
                || this->environmentConfig.targetConfig.deltaProgramming
           )
            && this->target->isProgramMemory(
                registeredBreakpoint.addressSpaceDescriptor,
                registeredBreakpoint.memorySegmentDescriptor,
                registeredBreakpoint.address,
                registeredBreakpoint.size
            )
        ) {
            auto& cache = this->getProgramMemoryCache(registeredBreakpoint.memorySegmentDescriptor);
            if (cache.contains(registeredBreakpoint.address, registeredBreakpoint.size)) {
                // Update program memory cache with the original instruction
                cache.insert(
                    registeredBreakpoint.address,
                    TargetMemoryBufferSpan{
                        registeredBreakpoint.originalData.begin(),
                        registeredBreakpoint.originalData.begin() + registeredBreakpoint.size
                    }
                );
            }
        }

        registry.remove(registeredBreakpoint);
    }

    void TargetControllerComponent::clearAllBreakpoints() {
        for (const auto& [addressSpaceId, breakpointsByAddress] : this->softwareBreakpointRegistry) {
            for (const auto& [address, breakpoint] : breakpointsByAddress) {
                this->removeProgramBreakpoint(breakpoint);
            }
        }

        for (const auto& [addressSpaceId, breakpointsByAddress] : this->hardwareBreakpointRegistry) {
            for (const auto& [address, breakpoint] : breakpointsByAddress) {
                this->removeProgramBreakpoint(breakpoint);
            }
        }
    }

    void TargetControllerComponent::enableProgrammingMode() {
        Logger::debug("Enabling programming mode");
        this->target->enableProgrammingMode();
        Logger::warning("Programming mode enabled");

        if (this->environmentConfig.targetConfig.deltaProgramming && this->deltaProgrammingInterface != nullptr) {
            this->deltaProgrammingSession = DeltaProgramming::Session{};
        }

        auto newState = *(this->targetState);
        newState.mode = TargetMode::PROGRAMMING;
        this->updateTargetState(newState);
    }

    void TargetControllerComponent::disableProgrammingMode() {
        if (this->deltaProgrammingSession.has_value()) {
            this->commitDeltaProgrammingSession(*(this->deltaProgrammingSession));
            this->deltaProgrammingSession = std::nullopt;
        }

        Logger::debug("Disabling programming mode");
        this->target->disableProgrammingMode();
        Logger::info("Programming mode disabled");

        Logger::info("Restoring breakpoints");
        this->target->stop();

        static const auto refreshOriginalData = [this] (TargetProgramBreakpoint& breakpoint) {
            const auto originalData = this->readTargetMemory(
                breakpoint.addressSpaceDescriptor,
                breakpoint.memorySegmentDescriptor,
                breakpoint.address,
                breakpoint.size,
                {},
                true
            );

            std::copy(originalData.begin(), originalData.end(), breakpoint.originalData.begin());
        };

        for (auto& [addressSpaceId, breakpointsByAddress] : this->softwareBreakpointRegistry) {
            for (auto& [address, breakpoint] : breakpointsByAddress) {
                refreshOriginalData(breakpoint);
                this->target->setProgramBreakpoint(breakpoint);

                auto& cache = this->getProgramMemoryCache(breakpoint.memorySegmentDescriptor);
                cache.insert(
                    breakpoint.address,
                    this->target->readMemory(
                        breakpoint.addressSpaceDescriptor,
                        breakpoint.memorySegmentDescriptor,
                        breakpoint.address,
                        breakpoint.size,
                        {}
                    )
                );
            }
        }

        for (auto& [addressSpaceId, breakpointsByAddress] : this->hardwareBreakpointRegistry) {
            for (auto& [address, breakpoint] : breakpointsByAddress) {
                refreshOriginalData(breakpoint);
                this->target->setProgramBreakpoint(breakpoint);
            }
        }

        auto newState = *(this->targetState);
        newState.mode = TargetMode::DEBUGGING;
        newState.executionState = TargetExecutionState::STOPPED;
        this->updateTargetState(newState);
    }

    TargetMemoryCache& TargetControllerComponent::getProgramMemoryCache(
        const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor
    ) {
        auto cacheIt = this->programMemoryCachesBySegmentId.find(memorySegmentDescriptor.id);

        if (cacheIt == this->programMemoryCachesBySegmentId.end()) {
            cacheIt = this->programMemoryCachesBySegmentId.emplace(
                memorySegmentDescriptor.id,
                TargetMemoryCache{memorySegmentDescriptor}
            ).first;
        }

        return cacheIt->second;
    }

    void TargetControllerComponent::commitDeltaProgrammingSession(const DeltaProgramming::Session& session) {
        using Services::AlignmentService;
        using Services::StringService;

        /*
         * If a single write operation cannot be committed, we must abandon the whole session.
         *
         * We prepare CommitOperation objects and then commit all of them at the end, allowing for the abandoning of
         * the session, if necessary.
         */
        struct CommitOperation
        {
            const TargetAddressSpaceDescriptor& addressSpaceDescriptor;
            const TargetMemorySegmentDescriptor& memorySegmentDescriptor;
            std::vector<DeltaProgramming::Session::WriteOperation::Region> deltaSegments;
        };
        auto commitOperations = std::vector<CommitOperation>{};

        for (const auto& [segmentId, writeOperation] : session.writeOperationsBySegmentId) {
            auto& segmentCache = this->getProgramMemoryCache(writeOperation.memorySegmentDescriptor);

            // Can the program memory cache facilitate diffing with all regions in this write operation?
            for (const auto& region : writeOperation.regions) {
                if (!segmentCache.contains(region.addressRange.startAddress, region.addressRange.size())) {
                    Logger::info("Abandoning delta programming session - insufficient data in program memory cache");
                    return this->abandonDeltaProgrammingSession(session);
                }
            }

            const auto alignTo = this->deltaProgrammingInterface->deltaBlockSize(
                writeOperation.addressSpaceDescriptor,
                writeOperation.memorySegmentDescriptor
            );

            /*
             * Ensure that the segment cache has sufficient data to facilitate delta segment alignment
             *
             * If the cache doesn't contain the necessary data for alignment, we just fill it with 0xFF instead of
             * obtaining the data with a read operation. This saves us some time.
             */
            for (const auto& region : writeOperation.regions) {
                const auto alignedAddress = AlignmentService::alignMemoryAddress(
                    region.addressRange.startAddress,
                    alignTo
                );
                const auto alignedSize = AlignmentService::alignMemorySize(
                    region.addressRange.size() + (region.addressRange.startAddress - alignedAddress),
                    alignTo
                );

                if (!segmentCache.contains(alignedAddress, alignedSize)) {
                    if (region.addressRange.startAddress != alignedAddress) {
                        segmentCache.fill(alignedAddress, region.addressRange.startAddress - alignedAddress, 0xFF);
                    }

                    if (region.addressRange.size() != alignedSize) {
                        segmentCache.fill(
                            region.addressRange.endAddress + 1,
                            alignedSize - region.addressRange.size(),
                            0xFF
                        );
                    }
                }
            }

            /*
             * When constructing the delta segments, any software breakpoints currently installed in this memory
             * segment can interfere with the diffing of the new program and the program memory cache.
             *
             * For this reason, we make a copy of the program memory cache and strip any software breakpoints from it,
             * before constructing the delta segments.
             */
            auto cacheData = segmentCache.data;
            for (const auto& [addressSpaceId, breakpointsByAddress] : this->softwareBreakpointRegistry) {
                for (const auto& [address, breakpoint] : breakpointsByAddress) {
                    if (breakpoint.memorySegmentDescriptor != writeOperation.memorySegmentDescriptor) {
                        continue;
                    }

                    std::copy(
                        breakpoint.originalData.begin(),
                        breakpoint.originalData.begin() + breakpoint.size,
                        cacheData.begin() + (breakpoint.address
                            - breakpoint.memorySegmentDescriptor.addressRange.startAddress)
                    );
                }
            }

            auto operation = CommitOperation{
                .addressSpaceDescriptor = writeOperation.addressSpaceDescriptor,
                .memorySegmentDescriptor = writeOperation.memorySegmentDescriptor,
                .deltaSegments = writeOperation.deltaSegments(cacheData, alignTo)
            };

            if (operation.deltaSegments.empty()) {
                Logger::warning("Abandoning delta programming session - zero delta segments");
                return this->abandonDeltaProgrammingSession(session);
            }

            if (
                this->deltaProgrammingInterface->shouldAbandonSession(
                    operation.addressSpaceDescriptor,
                    operation.memorySegmentDescriptor,
                    operation.deltaSegments
                )
            ) {
                Logger::info("Abandoning delta programming session - upon target driver request");
                return this->abandonDeltaProgrammingSession(session);
            }

            commitOperations.emplace_back(std::move(operation));
        }

        Logger::info("Committing delta programming session");

        for (const auto& operation : commitOperations) {
            auto& segmentCache = this->getProgramMemoryCache(operation.memorySegmentDescriptor);

            Logger::info(
                std::to_string(operation.deltaSegments.size()) + " delta segment(s) to be flushed to "
                    + StringService::formatKey(operation.memorySegmentDescriptor.key)
            );
            for (const auto& deltaSegment : operation.deltaSegments) {
                Logger::info(
                    "Flushing delta segment 0x" + StringService::toHex(deltaSegment.addressRange.startAddress)
                        + " -> 0x" + StringService::toHex(deltaSegment.addressRange.endAddress) + " - "
                        + std::to_string(deltaSegment.buffer.size()) + " byte(s)"
                );
                this->target->writeMemory(
                    operation.addressSpaceDescriptor,
                    operation.memorySegmentDescriptor,
                    deltaSegment.addressRange.startAddress,
                    deltaSegment.buffer
                );

                segmentCache.insert(deltaSegment.addressRange.startAddress, deltaSegment.buffer);
            }
        }
    }

    void TargetControllerComponent::abandonDeltaProgrammingSession(const DeltaProgramming::Session& session) {
        for (const auto& [segmentId, eraseOperation] : session.eraseOperationsBySegmentId) {
            this->eraseTargetMemory(eraseOperation.addressSpaceDescriptor, eraseOperation.memorySegmentDescriptor);
        }

        for (const auto& [segmentId, writeOperation] : session.writeOperationsBySegmentId) {
            for (const auto& region : writeOperation.regions) {
                this->writeTargetMemory(
                    writeOperation.addressSpaceDescriptor,
                    writeOperation.memorySegmentDescriptor,
                    region.addressRange.startAddress,
                    region.buffer
                );
            }
        }
    }

    void TargetControllerComponent::onShutdownTargetControllerEvent(const Events::ShutdownTargetController&) {
        this->shutdown();
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

        if (
            this->targetState->mode == TargetMode::PROGRAMMING
            && this->deltaProgrammingSession.has_value()
            && this->target->isProgramMemory(
                command.addressSpaceDescriptor,
                command.memorySegmentDescriptor,
                command.startAddress,
                static_cast<TargetMemorySize>(command.buffer.size())
            )
        ) {
            Logger::debug(
                "Pushing program memory write operation, at 0x" + Services::StringService::toHex(command.startAddress)
                    + ", " + std::to_string(command.buffer.size()) + " byte(s), to active delta programming session"
            );

            this->deltaProgrammingSession->pushWriteOperation(
                command.addressSpaceDescriptor,
                command.memorySegmentDescriptor,
                command.startAddress,
                std::move(command.buffer)
            );

            return std::make_unique<Response>();
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
        if (
            this->targetState->mode == TargetMode::PROGRAMMING
            && this->deltaProgrammingSession.has_value()
            && this->target->isProgramMemory(
                command.addressSpaceDescriptor,
                command.memorySegmentDescriptor,
                command.memorySegmentDescriptor.addressRange.startAddress,
                command.memorySegmentDescriptor.addressRange.size()
            )
        ) {
            Logger::debug("Pushing program memory erase operation to active delta programming session");

            this->deltaProgrammingSession->pushEraseOperation(
                command.addressSpaceDescriptor,
                command.memorySegmentDescriptor
            );

            return std::make_unique<Response>();
        }

        this->eraseTargetMemory(command.addressSpaceDescriptor, command.memorySegmentDescriptor);
        return std::make_unique<Response>();
    }

    std::unique_ptr<Response> TargetControllerComponent::handleStepTargetExecution(StepTargetExecution&) {
        this->stepTarget();
        return std::make_unique<Response>();
    }

    std::unique_ptr<ProgramBreakpoint> TargetControllerComponent::handleSetProgramBreakpointBreakpointAnyType(
        SetProgramBreakpointAnyType& command
    ) {
        if (command.size > TargetProgramBreakpoint::MAX_SIZE) {
            throw Exception{"Invalid breakpoint size"};
        }

        auto breakpoint = TargetProgramBreakpoint{
            .addressSpaceDescriptor = command.addressSpaceDescriptor,
            .memorySegmentDescriptor = command.memorySegmentDescriptor,
            .address = command.address,
            .size = command.size,
            .type = this->environmentConfig.targetConfig.hardwareBreakpoints && this->availableHardwareBreakpoints() > 0
                ? TargetProgramBreakpoint::Type::HARDWARE
                : TargetProgramBreakpoint::Type::SOFTWARE,
            .originalData = {}
        };

        const auto originalData = this->readTargetMemory(
            command.addressSpaceDescriptor,
            command.memorySegmentDescriptor,
            command.address,
            command.size,
            {},
            true
        );

        std::copy(originalData.begin(), originalData.end(), breakpoint.originalData.begin());

        this->setProgramBreakpoint(breakpoint);
        return std::make_unique<ProgramBreakpoint>(breakpoint);
    }

    std::unique_ptr<Response> TargetControllerComponent::handleRemoveProgramBreakpoint(
        RemoveProgramBreakpoint& command
    ) {
        this->removeProgramBreakpoint(command.breakpoint);
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

    std::unique_ptr<TargetPassthroughHelpText> TargetControllerComponent::handleTargetPassthroughHelpText(
        GetTargetPassthroughHelpText& command
    ) {
        return std::make_unique<TargetPassthroughHelpText>(this->target->passthroughCommandHelpText());
    }

    std::unique_ptr<TargetPassthroughResponse> TargetControllerComponent::handleTargetPassthroughCommand(
        InvokeTargetPassthroughCommand& command
    ) {
        return std::make_unique<TargetPassthroughResponse>(this->target->invokePassthroughCommand(command.command));
    }
}
