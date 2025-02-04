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
#include "src/Helpers/Synchronised.hpp"
#include "src/Helpers/ConditionVariableNotifier.hpp"

#include "TargetControllerState.hpp"
#include "AtomicSession.hpp"

// Commands
#include "Commands/Command.hpp"
#include "Commands/StartAtomicSession.hpp"
#include "Commands/EndAtomicSession.hpp"
#include "Commands/Shutdown.hpp"
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
#include "Commands/SetProgramBreakpointAnyType.hpp"
#include "Commands/RemoveProgramBreakpoint.hpp"
#include "Commands/SetTargetProgramCounter.hpp"
#include "Commands/SetTargetStackPointer.hpp"
#include "Commands/GetTargetGpioPadStates.hpp"
#include "Commands/SetTargetGpioPadState.hpp"
#include "Commands/GetTargetStackPointer.hpp"
#include "Commands/GetTargetProgramCounter.hpp"
#include "Commands/EnableProgrammingMode.hpp"
#include "Commands/DisableProgrammingMode.hpp"
#include "Commands/GetTargetPassthroughHelpText.hpp"
#include "Commands/InvokeTargetPassthroughCommand.hpp"

// Responses
#include "Responses/Response.hpp"
#include "Responses/AtomicSessionId.hpp"
#include "Responses/TargetDescriptor.hpp"
#include "Responses/TargetState.hpp"
#include "Responses/TargetRegistersRead.hpp"
#include "Responses/TargetMemoryRead.hpp"
#include "Responses/TargetGpioPadStates.hpp"
#include "Responses/TargetStackPointer.hpp"
#include "Responses/TargetProgramCounter.hpp"
#include "Responses/ProgramBreakpoint.hpp"
#include "Responses/TargetPassthroughHelpText.hpp"
#include "Responses/TargetPassthroughResponse.hpp"

#include "src/DebugToolDrivers/DebugTools.hpp"
#include "src/Targets/BriefTargetDescriptor.hpp"
#include "src/Targets/Target.hpp"
#include "src/Targets/Targets.hpp"
#include "src/Targets/TargetRegisterDescriptor.hpp"
#include "src/Targets/TargetMemory.hpp"
#include "src/Targets/TargetMemoryAddressRange.hpp"
#include "src/Targets/TargetAddressSpaceDescriptor.hpp"
#include "src/Targets/TargetMemorySegmentDescriptor.hpp"
#include "src/Targets/ProgramBreakpointRegistry.hpp"
#include "src/Targets/TargetMemoryCache.hpp"
#include "src/Targets/DeltaProgramming/DeltaProgrammingInterface.hpp"
#include "src/Targets/DeltaProgramming/Session.hpp"

#include "src/EventManager/EventManager.hpp"
#include "src/EventManager/EventListener.hpp"
#include "src/EventManager/Events/Events.hpp"

namespace TargetController
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

        static void registerCommand(
            std::unique_ptr<Commands::Command> command,
            const std::optional<AtomicSessionIdType>& atomicSessionId
        );

        static std::optional<std::unique_ptr<Responses::Response>> waitForResponse(
            Commands::CommandIdType commandId,
            std::optional<std::chrono::milliseconds> timeout = std::nullopt
        );

    private:
        static inline Synchronised<std::queue<std::unique_ptr<Commands::Command>>> commandQueue;

        /**
         * We have a dedicated queue for atomic sessions.
         *
         * During an atomic session, all commands for the session are placed into this dedicated queue.
         * The TargetController will only serve commands from this dedicated queue, until the atomic session ends.
         */
        static inline Synchronised<std::queue<std::unique_ptr<Commands::Command>>> atomicSessionCommandQueue;

        static inline Synchronised<
            std::map<Commands::CommandIdType, std::unique_ptr<Responses::Response>>
        > responsesByCommandId;

        static inline ConditionVariableNotifier notifier = {};
        static inline std::condition_variable responsesByCommandIdCv = {};

        static inline std::atomic<TargetControllerState> state = TargetControllerState::INACTIVE;

        ProjectConfig projectConfig;
        EnvironmentConfig environmentConfig;

        std::optional<AtomicSession> activeAtomicSession = std::nullopt;

        std::unique_ptr<DebugTool> debugTool = nullptr;
        std::unique_ptr<Targets::Target> target = nullptr;

        Targets::DeltaProgramming::DeltaProgrammingInterface* deltaProgrammingInterface = nullptr;

        std::map<
            Commands::CommandType,
            std::function<std::unique_ptr<Responses::Response>(Commands::Command&)>
        > commandHandlersByCommandType;

        EventListenerPointer eventListener = std::make_shared<EventListener>("TargetControllerEventListener");

        std::unique_ptr<const Targets::TargetDescriptor> targetDescriptor = nullptr;
        std::unique_ptr<Targets::TargetState> targetState = nullptr;

        /**
         * Target register descriptors mapped by the address space key
         */
        std::map<std::string, Targets::TargetRegisterDescriptors> registerDescriptorsByAddressSpaceKey;

        Targets::ProgramBreakpointRegistry softwareBreakpointRegistry;
        Targets::ProgramBreakpointRegistry hardwareBreakpointRegistry;

        /**
         * The target's program memory cache
         *
         * If program caching is enabled, all program memory reads will be serviced by the cache, if we have the data.
         *
         * Most targets only have a single memory segment for program memory, but some may have multiple program
         * memories, across multiple address spaces. We have a single cache for each memory segment.
         */
        std::map<Targets::TargetMemorySegmentId, Targets::TargetMemoryCache> programMemoryCachesBySegmentId;

        /**
         * Active delta programming session
         */
        std::optional<Targets::DeltaProgramming::Session> deltaProgrammingSession;

        /**
         * Registers a handler function for a particular command type.
         * Only one handler function can be registered per command type.
         *
         * @tparam CommandType
         * @param callback
         */
        template<class CommandType>
        void registerCommandHandler(std::function<std::unique_ptr<Responses::Response>(CommandType&)> callback) {
            this->commandHandlersByCommandType.emplace(
                CommandType::type,
                [callback] (Commands::Command& command) {
                    // Downcast the command to the expected type
                    return callback(dynamic_cast<CommandType&>(command));
                }
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
         * Exit point - must be called before the TargetController thread is terminated.
         *
         * Handles releasing the hardware among other clean-up related things.
         */
        void shutdown();

        /**
         * Constructs a mapping of supported debug tool names to lambdas. The lambdas should *only* instantiate
         * and return an instance to the derived DebugTool class. They should not attempt to establish
         * a connection to the device.
         *
         * @return
         */
        std::map<std::string, std::function<std::unique_ptr<DebugTool>()>> getSupportedDebugTools();

        /**
         * Constructs a Target instance from a BriefTargetDescriptor object.
         *
         * @param briefDescriptor
         * @return
         */
        std::unique_ptr<Targets::Target> constructTarget(const Targets::BriefTargetDescriptor& briefDescriptor);

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
         * Establishes a connection with the debug tool and target. Prepares the hardware for a debug session.
         */
        void acquireHardware();

        /**
         * Attempts to gracefully disconnect from the debug tool and the target. All control of the debug tool and
         * target will cease.
         */
        void releaseHardware();

        void startAtomicSession();
        void endActiveAtomicSession();

        void refreshExecutionState(bool forceUpdate = false);
        void updateTargetState(const Targets::TargetState& newState);

        void stopTarget();
        void resumeTarget();
        void stepTarget();
        void resetTarget();

        Targets::TargetRegisterDescriptorAndValuePairs readTargetRegisters(
            const Targets::TargetRegisterDescriptors& descriptors
        );

        void writeTargetRegisters(const Targets::TargetRegisterDescriptorAndValuePairs& registers);

        Targets::TargetMemoryBuffer readTargetMemory(
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            Targets::TargetMemoryAddress startAddress,
            Targets::TargetMemorySize bytes,
            const std::set<Targets::TargetMemoryAddressRange>& excludedAddressRanges,
            bool bypassCache
        );

        void writeTargetMemory(
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            Targets::TargetMemoryAddress startAddress,
            Targets::TargetMemoryBufferSpan buffer
        );

        void eraseTargetMemory(
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor
        );

        std::uint32_t availableHardwareBreakpoints();
        void setProgramBreakpoint(const Targets::TargetProgramBreakpoint& breakpoint);
        void removeProgramBreakpoint(const Targets::TargetProgramBreakpoint& breakpoint);
        void clearAllBreakpoints();

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
         * Fetches the program memory cache object for the given memory segment. If the segment has no associated
         * cache object, one will be created.
         *
         * @param memorySegmentDescriptor
         * @return
         */
        Targets::TargetMemoryCache& getProgramMemoryCache(
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor
        );

        void commitDeltaProgrammingSession(const Targets::DeltaProgramming::Session& session);
        void abandonDeltaProgrammingSession(const Targets::DeltaProgramming::Session& session);

        /**
         * Invokes a shutdown.
         *
         * @param event
         */
        void onShutdownTargetControllerEvent(const Events::ShutdownTargetController& event);

        // Command handlers
        std::unique_ptr<Responses::AtomicSessionId> handleStartAtomicSession(Commands::StartAtomicSession& command);
        std::unique_ptr<Responses::Response> handleEndAtomicSession(Commands::EndAtomicSession& command);
        std::unique_ptr<Responses::Response> handleShutdown(Commands::Shutdown& command);
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
        std::unique_ptr<Responses::ProgramBreakpoint> handleSetProgramBreakpointBreakpointAnyType(
            Commands::SetProgramBreakpointAnyType& command
        );
        std::unique_ptr<Responses::Response> handleRemoveProgramBreakpoint(Commands::RemoveProgramBreakpoint& command);
        std::unique_ptr<Responses::Response> handleSetProgramCounter(Commands::SetTargetProgramCounter& command);
        std::unique_ptr<Responses::Response> handleSetStackPointer(Commands::SetTargetStackPointer& command);
        std::unique_ptr<Responses::TargetGpioPadStates> handleGetTargetGpioPadStates(
            Commands::GetTargetGpioPadStates& command
        );
        std::unique_ptr<Responses::Response> handleSetTargetGpioPadState(Commands::SetTargetGpioPadState& command);
        std::unique_ptr<Responses::TargetStackPointer> handleGetTargetStackPointer(
            Commands::GetTargetStackPointer& command
        );
        std::unique_ptr<Responses::TargetProgramCounter> handleGetTargetProgramCounter(
            Commands::GetTargetProgramCounter& command
        );
        std::unique_ptr<Responses::Response> handleEnableProgrammingMode(Commands::EnableProgrammingMode& command);
        std::unique_ptr<Responses::Response> handleDisableProgrammingMode(Commands::DisableProgrammingMode& command);
        std::unique_ptr<Responses::TargetPassthroughHelpText> handleTargetPassthroughHelpText(
            Commands::GetTargetPassthroughHelpText& command
        );
        std::unique_ptr<Responses::TargetPassthroughResponse> handleTargetPassthroughCommand(
            Commands::InvokeTargetPassthroughCommand& command
        );
    };
}
