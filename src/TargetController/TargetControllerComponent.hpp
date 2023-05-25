#pragma once

#include <atomic>
#include <memory>
#include <queue>
#include <condition_variable>
#include <optional>
#include <chrono>
#include <map>
#include <string>
#include <functional>
#include <QJsonObject>
#include <QJsonArray>

#include "src/Helpers/Thread.hpp"
#include "src/Helpers/SyncSafe.hpp"
#include "src/Helpers/ConditionVariableNotifier.hpp"

#include "TargetControllerState.hpp"

// Commands
#include "Commands/Command.hpp"
#include "Commands/GetState.hpp"
#include "Commands/Resume.hpp"
#include "Commands/Suspend.hpp"
#include "Commands/GetTargetDescriptor.hpp"
#include "Commands/GetTargetState.hpp"
#include "Commands/StopTargetExecution.hpp"
#include "Commands/ResumeTargetExecution.hpp"
#include "Commands/ResetTarget.hpp"
#include "Commands/ReadTargetRegisters.hpp"
#include "Commands/WriteTargetRegisters.hpp"
#include "Commands/ReadTargetMemory.hpp"
#include "Commands/WriteTargetMemory.hpp"
#include "Commands/EraseTargetMemory.hpp"
#include "Commands/StepTargetExecution.hpp"
#include "Commands/SetBreakpoint.hpp"
#include "Commands/RemoveBreakpoint.hpp"
#include "Commands/SetTargetProgramCounter.hpp"
#include "Commands/GetTargetPinStates.hpp"
#include "Commands/SetTargetPinState.hpp"
#include "Commands/GetTargetStackPointer.hpp"
#include "Commands/GetTargetProgramCounter.hpp"
#include "Commands/EnableProgrammingMode.hpp"
#include "Commands/DisableProgrammingMode.hpp"

// Responses
#include "Responses/Response.hpp"
#include "Responses/State.hpp"
#include "Responses/TargetDescriptor.hpp"
#include "Responses/TargetState.hpp"
#include "Responses/TargetRegistersRead.hpp"
#include "Responses/TargetMemoryRead.hpp"
#include "Responses/TargetPinStates.hpp"
#include "Responses/TargetStackPointer.hpp"
#include "Responses/TargetProgramCounter.hpp"

#include "src/DebugToolDrivers/DebugTools.hpp"
#include "src/Targets/Target.hpp"
#include "src/Targets/Targets.hpp"
#include "src/Targets/TargetRegister.hpp"
#include "src/Targets/TargetMemory.hpp"

#include "src/EventManager/EventManager.hpp"
#include "src/EventManager/EventListener.hpp"
#include "src/EventManager/Events/Events.hpp"

namespace Bloom::TargetController
{
    static_assert(std::atomic<TargetControllerState>::is_always_lock_free);

    /**
     * The TargetController possesses full control of the debugging target and the debug tool.
     *
     * The TargetController runs on a dedicated thread. Its sole purpose is to handle communication to & from the
     * debug tool and target.
     *
     * The TargetController should be oblivious to any manufacture/device specific functionality. It should
     * only ever interface with the base Target and DebugTool classes.
     */
    class TargetControllerComponent: public Thread
    {
    public:
        explicit TargetControllerComponent(
            const ProjectConfig& projectConfig,
            const EnvironmentConfig& environmentConfig
        );

        /**
         * Entry point for the TargetController.
         */
        void run();

        static void registerCommand(std::unique_ptr<Commands::Command> command);

        static std::optional<std::unique_ptr<Responses::Response>> waitForResponse(
            Commands::CommandIdType commandId,
            std::optional<std::chrono::milliseconds> timeout = std::nullopt
        );

    private:
        static inline SyncSafe<
            std::queue<std::unique_ptr<Commands::Command>>
        > commandQueue;

        static inline SyncSafe<
            std::map<Commands::CommandIdType, std::unique_ptr<Responses::Response>>
        > responsesByCommandId;

        static inline ConditionVariableNotifier notifier = ConditionVariableNotifier();
        static inline std::condition_variable responsesByCommandIdCv = std::condition_variable();

        /**
         * The TC starts off in a suspended state. TargetControllerComponent::resume() is invoked from the start up
         * routine.
         */
        TargetControllerState state = TargetControllerState::SUSPENDED;

        ProjectConfig projectConfig;
        EnvironmentConfig environmentConfig;

        /**
         * The TargetController should be the sole owner of the target and debugTool. They are constructed and
         * destroyed within the TargetController. Under no circumstance should ownership of these resources be
         * transferred to any other component within Bloom.
         */
        std::unique_ptr<Targets::Target> target = nullptr;
        std::unique_ptr<DebugTool> debugTool = nullptr;

        std::map<
            Commands::CommandType,
            std::function<std::unique_ptr<Responses::Response>(Commands::Command&)>
        > commandHandlersByCommandType;

        EventListenerPointer eventListener = std::make_shared<EventListener>("TargetControllerEventListener");

        /**
         * We keep record of the last known execution state of the target. When the connected target reports a
         * different state to what's stored in lastTargetState, a state change (TargetExecutionStopped/TargetExecutionResumed)
         * event is emitted.
         */
        Targets::TargetState lastTargetState = Targets::TargetState::UNKNOWN;

        std::optional<const Targets::TargetDescriptor> targetDescriptor;

        /**
         * Target register descriptors mapped by the memory type on which the register is stored.
         */
        std::map<Targets::TargetMemoryType, Targets::TargetRegisterDescriptors> registerDescriptorsByMemoryType;

        /**
         * Memory address ranges for target registers, mapped by the register memory type.
         */
        std::map<Targets::TargetMemoryType, Targets::TargetMemoryAddressRange> registerAddressRangeByMemoryType;

        /**
         * Registers a handler function for a particular command type.
         * Only one handler function can be registered per command type.
         *
         * @tparam CommandType
         * @param callback
         */
        template<class CommandType>
        void registerCommandHandler(std::function<std::unique_ptr<Responses::Response>(CommandType&)> callback) {
            this->commandHandlersByCommandType.insert(
                std::pair(
                    CommandType::type,
                    [callback] (Commands::Command& command) {
                        // Downcast the command to the expected type
                        return callback(dynamic_cast<CommandType&>(command));
                    }
                )
            );
        }

        /**
         * Removes any registered handler for a given command type. After calling this function, any commands issued
         * for the given command type will be rejected with a "No handler registered for this command." error, until a
         * handler is registered again.
         *
         * @param commandType
         */
        void deregisterCommandHandler(Commands::CommandType commandType);

        /**
         * Updates the state of the TargetController and emits a state changed event.
         *
         * @param state
         * @param emitEvent
         */
        void setThreadStateAndEmitEvent(ThreadState state) {
            this->threadState = state;
            EventManager::triggerEvent(
                std::make_shared<Events::TargetControllerThreadStateChanged>(state)
            );
        }

        /**
         * Because the TargetController hogs the thread, this method must be called in a dedicated thread.
         */
        void startup();

        /**
         * Constructs a mapping of supported debug tool names to lambdas. The lambdas should *only* instantiate
         * and return an instance to the derived DebugTool class. They should not attempt to establish
         * a connection to the device.
         *
         * @return
         */
        std::map<std::string, std::function<std::unique_ptr<DebugTool>()>> getSupportedDebugTools();

        /**
         * Constructs a mapping of supported target names to lambdas. The lambdas should instantiate and return an
         * instance to the appropriate Target class.
         *
         * @return
         */
        std::map<std::string, std::function<std::unique_ptr<Targets::Target>(const TargetConfig&)>> getSupportedTargets();

        /**
         * Processes any pending commands in the queue.
         */
        void processQueuedCommands();

        /**
         * Records a response for a given command ID. Notifies the TargetControllerComponent::responsesByCommandIdCv
         * condition variable of the new response.
         *
         * @param commandId
         * @param response
         */
        void registerCommandResponse(Commands::CommandIdType commandId, std::unique_ptr<Responses::Response> response);

        /**
         * Exit point - must be called before the TargetController thread is terminated.
         *
         * Handles releasing the hardware among other clean-up related things.
         */
        void shutdown();

        /**
         * Puts the TargetController into the suspended state.
         *
         * In this state, the hardware is released and the TargetController will only handle a subset of events.
         */
        void suspend();

        /**
         * Wakes the TargetController from the suspended state.
         */
        void resume();

        /**
         * Establishes a connection with the debug tool and target. Prepares the hardware for a debug session.
         */
        void acquireHardware();

        /**
         * Attempts to gracefully disconnect from the debug tool and the target. All control of the debug tool and
         * target will cease.
         */
        void releaseHardware();

        /**
         * Extracts address ranges and groups target register descriptors.
         */
        void loadRegisterDescriptors();

        /**
         * Resolves the descriptors of all target registers found within the given address range and memory type.
         *
         * @param startAddress
         * @param endAddress
         * @param memoryType
         * @return
         */
        Targets::TargetRegisterDescriptors getRegisterDescriptorsWithinAddressRange(
            Targets::TargetMemoryAddress startAddress,
            Targets::TargetMemoryAddress endAddress,
            Targets::TargetMemoryType memoryType
        );

        /**
         * Should fire any events queued on the target.
         */
        void fireTargetEvents();

        /**
         * Triggers a target reset and emits a TargetReset event.
         */
        void resetTarget();

        /**
         * Puts the target into programming mode and disables command handlers for debug commands (commands that serve
         * debug operations such as SetBreakpoint, ResumeTargetExecution, etc).
         */
        void enableProgrammingMode();

        /**
         * Pulls the target out of programming mode and enables command handlers for debug commands.
         */
        void disableProgrammingMode();

        /**
         * Returns a cached instance of the target's TargetDescriptor.
         *
         * @return
         */
        const Targets::TargetDescriptor& getTargetDescriptor();

        /**
         * Invokes a shutdown.
         *
         * @param event
         */
        void onShutdownTargetControllerEvent(const Events::ShutdownTargetController& event);

        /**
         * Will hold the target stopped at it's current state.
         *
         * @param event
         */
        void onDebugSessionStartedEvent(const Events::DebugSessionStarted& event);

        /**
         * Will simply kick off execution on the target.
         *
         * @param event
         */
        void onDebugSessionFinishedEvent(const Events::DebugSessionFinished& event);

        // Command handlers
        std::unique_ptr<Responses::State> handleGetState(Commands::GetState& command);
        std::unique_ptr<Responses::Response> handleSuspend(Commands::Suspend& command);
        std::unique_ptr<Responses::Response> handleResume(Commands::Resume& command);
        std::unique_ptr<Responses::TargetDescriptor> handleGetTargetDescriptor(Commands::GetTargetDescriptor& command);
        std::unique_ptr<Responses::TargetState> handleGetTargetState(Commands::GetTargetState& command);
        std::unique_ptr<Responses::Response> handleStopTargetExecution(Commands::StopTargetExecution& command);
        std::unique_ptr<Responses::Response> handleResumeTargetExecution(Commands::ResumeTargetExecution& command);
        std::unique_ptr<Responses::Response> handleResetTarget(Commands::ResetTarget& command);
        std::unique_ptr<Responses::TargetRegistersRead> handleReadTargetRegisters(
            Commands::ReadTargetRegisters& command
        );
        std::unique_ptr<Responses::Response> handleWriteTargetRegisters(Commands::WriteTargetRegisters& command);
        std::unique_ptr<Responses::TargetMemoryRead> handleReadTargetMemory(Commands::ReadTargetMemory& command);
        std::unique_ptr<Responses::Response> handleWriteTargetMemory(Commands::WriteTargetMemory& command);
        std::unique_ptr<Responses::Response> handleEraseTargetMemory(Commands::EraseTargetMemory& command);
        std::unique_ptr<Responses::Response> handleStepTargetExecution(Commands::StepTargetExecution& command);
        std::unique_ptr<Responses::Response> handleSetBreakpoint(Commands::SetBreakpoint& command);
        std::unique_ptr<Responses::Response> handleRemoveBreakpoint(Commands::RemoveBreakpoint& command);
        std::unique_ptr<Responses::Response> handleSetProgramCounter(Commands::SetTargetProgramCounter& command);
        std::unique_ptr<Responses::TargetPinStates> handleGetTargetPinStates(Commands::GetTargetPinStates& command);
        std::unique_ptr<Responses::Response> handleSetTargetPinState(Commands::SetTargetPinState& command);
        std::unique_ptr<Responses::TargetStackPointer> handleGetTargetStackPointer(
            Commands::GetTargetStackPointer& command
        );
        std::unique_ptr<Responses::TargetProgramCounter> handleGetTargetProgramCounter(
            Commands::GetTargetProgramCounter& command
        );
        std::unique_ptr<Responses::Response> handleEnableProgrammingMode(Commands::EnableProgrammingMode& command);
        std::unique_ptr<Responses::Response> handleDisableProgrammingMode(Commands::DisableProgrammingMode& command);
    };
}
