#include "TargetControllerComponent.hpp"

#include <thread>
#include <filesystem>
#include <typeindex>
#include <algorithm>

#include "Responses/Error.hpp"

#include "src/Helpers/Paths.hpp"
#include "src/Helpers/Process.hpp"
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

    using Commands::CommandIdType;

    using Commands::Command;
    using Commands::GetState;
    using Commands::Resume;
    using Commands::Suspend;
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
    using Responses::TargetRegistersRead;
    using Responses::TargetMemoryRead;
    using Responses::TargetPinStates;
    using Responses::TargetStackPointer;
    using Responses::TargetProgramCounter;

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
            Logger::error("TargetController failed to start up. See below for errors:");
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
        this->blockAllSignals();
        this->eventListener->setInterruptEventNotifier(&TargetControllerComponent::notifier);
        EventManager::registerListener(this->eventListener);

        // Install Bloom's udev rules if not already installed
        TargetControllerComponent::checkUdevRules();

        // Register command handlers
        this->registerCommandHandler<GetState>(
            std::bind(&TargetControllerComponent::handleGetState, this, std::placeholders::_1)
        );

        this->registerCommandHandler<Resume>(
            std::bind(&TargetControllerComponent::handleResume, this, std::placeholders::_1)
        );

        this->registerCommandHandler<Suspend>(
            std::bind(&TargetControllerComponent::handleSuspend, this, std::placeholders::_1)
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

        this->eventListener->registerCallbackForEventType<Events::DebugSessionStarted>(
            std::bind(&TargetControllerComponent::onDebugSessionStartedEvent, this, std::placeholders::_1)
        );

        this->resume();
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
            {
                "jtagice3",
                [] {
                    return std::make_unique<DebugToolDrivers::JtagIce3>();
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
        const auto avr8PdMapping = Avr8TargetDescriptionFile::getTargetDescriptionMapping();

        for (auto mapIt = avr8PdMapping.begin(); mapIt != avr8PdMapping.end(); mapIt++) {
            // Each target signature maps to an array of targets, as numerous targets can possess the same signature.
            const auto targets = mapIt.value().toArray();

            for (auto targetIt = targets.begin(); targetIt != targets.end(); targetIt++) {
                const auto targetName = targetIt->toObject().find("targetName").value().toString()
                    .toLower().toStdString();
                const auto targetSignatureHex = mapIt.key().toLower().toStdString();

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
            const auto queueLock = TargetControllerComponent::commandQueue.acquireLock();
            commands.swap(TargetControllerComponent::commandQueue.getValue());
        }

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

                if (this->state != TargetControllerState::ACTIVE && command->requiresActiveState()) {
                    throw Exception("Command rejected - TargetController not in active state.");
                }

                if (this->state == TargetControllerState::ACTIVE) {
                    if (command->requiresStoppedTargetState() && this->lastTargetState != TargetState::STOPPED) {
                        throw Exception("Command rejected - command requires target execution to be stopped.");
                    }

                    if (this->target->programmingModeEnabled() && command->requiresDebugMode()) {
                        throw Exception(
                            "Command rejected - command cannot be serviced whilst the target is in programming mode."
                        );
                    }
                }

                this->registerCommandResponse(commandId, commandHandlerIt->second(*(command.get())));

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
        const auto responseMappingLock = TargetControllerComponent::responsesByCommandId.acquireLock();
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
            if (!Process::isRunningAsRoot()) {
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

        this->eventListener->deregisterCallbacksForEventType<Events::DebugSessionFinished>();

        this->lastTargetState = TargetState::UNKNOWN;
        this->cachedTargetDescriptor = std::nullopt;
        this->registerDescriptorsByMemoryType.clear();
        this->registerAddressRangeByMemoryType.clear();

        TargetControllerComponent::state = TargetControllerState::SUSPENDED;
        EventManager::triggerEvent(std::make_shared<TargetControllerStateChanged>(TargetControllerComponent::state));

        Logger::debug("TargetController suspended");
    }

    void TargetControllerComponent::resume() {
        this->acquireHardware();
        this->loadRegisterDescriptors();

        this->eventListener->registerCallbackForEventType<Events::DebugSessionFinished>(
            std::bind(&TargetControllerComponent::onDebugSessionFinishedEvent, this, std::placeholders::_1)
        );

        TargetControllerComponent::state = TargetControllerState::ACTIVE;
        EventManager::triggerEvent(
            std::make_shared<TargetControllerStateChanged>(TargetControllerComponent::state)
        );

        if (this->target->getState() != TargetState::RUNNING) {
            this->target->run();
            this->lastTargetState = TargetState::RUNNING;
        }
    }

    void TargetControllerComponent::acquireHardware() {
        auto debugToolName = this->environmentConfig.debugToolConfig.name;
        auto targetName = this->environmentConfig.targetConfig.name;

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

        // Initiate debug tool and target
        this->debugTool = debugToolIt->second();

        Logger::info("Connecting to debug tool");
        this->debugTool->init();

        Logger::info("Debug tool connected");
        Logger::info("Debug tool name: " + this->debugTool->getName());
        Logger::info("Debug tool serial: " + this->debugTool->getSerialNumber());

        this->target = targetIt->second();

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

            if (
                promotedTarget == nullptr
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
        const auto& targetDescriptor = this->getTargetDescriptor();

        for (const auto& [registerType, registerDescriptors] : targetDescriptor.registerDescriptorsByType) {
            for (const auto& registerDescriptor : registerDescriptors) {
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
                const auto& registerDescriptors = this->registerDescriptorsByMemoryType.at(memoryType);

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
    }

    const Targets::TargetDescriptor& TargetControllerComponent::getTargetDescriptor() {
        if (!this->cachedTargetDescriptor.has_value()) {
            this->cachedTargetDescriptor.emplace(this->target->getDescriptor());
        }

        return *this->cachedTargetDescriptor;
    }

    void TargetControllerComponent::onShutdownTargetControllerEvent(const Events::ShutdownTargetController&) {
        this->shutdown();
    }

    void TargetControllerComponent::onDebugSessionStartedEvent(const Events::DebugSessionStarted&) {
        if (TargetControllerComponent::state == TargetControllerState::SUSPENDED) {
            Logger::debug("Waking TargetController");

            this->resume();
            this->fireTargetEvents();
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

    std::unique_ptr<Responses::State> TargetControllerComponent::handleGetState(GetState& command) {
        return std::make_unique<Responses::State>(this->state);
    }

    std::unique_ptr<Responses::Response> TargetControllerComponent::handleResume(Resume& command) {
        if (this->state != TargetControllerState::ACTIVE) {
            this->resume();
        }

        return std::make_unique<Response>();
    }

    std::unique_ptr<Responses::Response> TargetControllerComponent::handleSuspend(Suspend& command) {
        if (this->state != TargetControllerState::SUSPENDED) {
            this->suspend();
        }

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
            if (command.fromProgramCounter.has_value()) {
                this->target->setProgramCounter(command.fromProgramCounter.value());
            }

            this->target->run();
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
        return std::make_unique<TargetRegistersRead>(this->target->readRegisters(command.descriptors));
    }

    std::unique_ptr<Response> TargetControllerComponent::handleWriteTargetRegisters(WriteTargetRegisters& command) {
        this->target->writeRegisters(command.registers);

        auto registersWrittenEvent = std::make_shared<Events::RegistersWrittenToTarget>();
        registersWrittenEvent->registers = command.registers;

        EventManager::triggerEvent(registersWrittenEvent);

        return std::make_unique<Response>();
    }

    std::unique_ptr<TargetMemoryRead> TargetControllerComponent::handleReadTargetMemory(ReadTargetMemory& command) {
        return std::make_unique<TargetMemoryRead>(this->target->readMemory(
            command.memoryType,
            command.startAddress,
            command.bytes,
            command.excludedAddressRanges
        ));
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
                        registerDescriptor,
                        TargetMemoryBuffer(bufferBeginIt, bufferBeginIt + registerSize)
                    ));
                }

                EventManager::triggerEvent(registersWrittenEvent);
            }
        }

        return std::make_unique<Response>();
    }

    std::unique_ptr<Response> TargetControllerComponent::handleEraseTargetMemory(EraseTargetMemory& command) {
        const auto& targetDescriptor = this->getTargetDescriptor();

        if (
            command.memoryType == this->getTargetDescriptor().programMemoryType
            && !this->target->programmingModeEnabled()
        ) {
            throw Exception("Cannot erase program memory - programming mode not enabled.");
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

    std::unique_ptr<Response> TargetControllerComponent::handleSetBreakpoint(SetBreakpoint& command) {
        this->target->setBreakpoint(command.breakpoint.address);
        return std::make_unique<Response>();
    }

    std::unique_ptr<Response> TargetControllerComponent::handleRemoveBreakpoint(RemoveBreakpoint& command) {
        this->target->removeBreakpoint(command.breakpoint.address);
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
