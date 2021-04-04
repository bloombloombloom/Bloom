#pragma once

#include <map>
#include <functional>
#include <cstdint>

#include "src/EventManager/Events/Events.hpp"
#include "src/EventManager/EventManager.hpp"
#include "src/Exceptions/DebugServerInterrupted.hpp"
#include "src/ApplicationConfig.hpp"
#include "src/Helpers/Thread.hpp"
#include "src/Targets/TargetRegister.hpp"
#include "src/Targets/TargetBreakpoint.hpp"

namespace Bloom::DebugServers
{
    using Targets::TargetRegister;
    using Targets::TargetRegisterDescriptor;
    using Targets::TargetRegisters;
    using Targets::TargetRegisterMap;
    using Targets::TargetMemoryBuffer;
    using Targets::TargetMemoryType;
    using Targets::TargetBreakpoint;

    /**
     * The DebugServer exposes the connected target to third-party debugging software such as IDEs.
     * The DebugServer runs on a dedicated thread which is kicked off shortly after the TargetController has been
     * started.
     *
     * All supported DebugServers should be derived from this class.
     *
     * Bloom currently only supports one DebugServer - the GdbRspDebugServer.
     */
    class DebugServer: public Thread
    {
    private:
        /**
         * Prepares the debug server thread and then calls init().
         *
         * Derived classes should not override this method - they should instead use init().
         */
        void startup();

        /**
         * Calls close() and updates the thread state.
         *
         * As with startup(), derived classes should not override this method. They should use close() instead.
         */
        void shutdown();

        /**
         * Updates the state of the DebugServer and emits a state changed event.
         *
         * @param state
         * @param emitEvent
         */
        void setStateAndEmitEvent(ThreadState state) {
            Thread::setState(state);
            this->eventManager.triggerEvent(
                std::make_shared<Events::DebugServerStateChanged>(state)
            );
        };

        /**
         * Handles a shutdown request.
         *
         * @param event
         */
        void onShutdownDebugServerEvent(EventPointer<Events::ShutdownDebugServer> event);

    protected:
        /**
         * Application-wide instance to EventManager
         */
        EventManager& eventManager;
        EventListenerPointer eventListener = std::make_shared<EventListener>("DebugServerEventListener");

        /**
         * Enables the interruption of any blocking file IO.
         */
        std::shared_ptr<EventNotifier> interruptEventNotifier = nullptr;

        ApplicationConfig applicationConfig;
        EnvironmentConfig environmentConfig;
        DebugServerConfig debugServerConfig;

        /**
         * Called on startup of the DebugServer thread. Derived classes should implement any initialisation work here.
         */
        virtual void init() = 0;

        /**
         * Called repeatedly in an infinite loop when the DebugServer is running.
         */
        virtual void serve() = 0;

        /**
         * Called on shutdown of the debug server.
         */
        virtual void close() = 0;

        /**
         * Requests the TargetController to halt execution on the target.
         */
        void stopTargetExecution();

        /**
         * Requests the TargetController to continue execution on the target.
         *
         * @param fromAddress
         */
        void continueTargetExecution(std::optional<std::uint32_t> fromAddress);

        /**
         * Requests the TargetController to step execution on the target.
         *
         * @param fromAddress
         */
        void stepTargetExecution(std::optional<std::uint32_t> fromAddress);

        /**
         * Requests the TargetController to read register values from the target.
         *
         * @param descriptors
         *  Descriptors of the registers to read.
         *
         * @return
         */
        TargetRegisters readGeneralRegistersFromTarget(TargetRegisterDescriptors descriptors);

        /**
         * Requests the TargetController to write register values to the target.
         *
         * @param registers
         */
        void writeGeneralRegistersToTarget(TargetRegisters registers);

        /**
         * Requests the TargetController to read memory from the target.
         *
         * @param memoryType
         * @param startAddress
         * @param bytes
         * @return
         */
        TargetMemoryBuffer readMemoryFromTarget(TargetMemoryType memoryType, std::uint32_t startAddress, std::uint32_t bytes);

        /**
         * Requests the TargetController to write memory to the target.
         *
         * @param memoryType
         * @param startAddress
         * @param buffer
         */
        void writeMemoryToTarget(TargetMemoryType memoryType, std::uint32_t startAddress, const TargetMemoryBuffer& buffer);

        /**
         * Requests the TargetController to set a breakpoint on the target.
         *
         * @param breakpoint
         */
        void setBreakpointOnTarget(TargetBreakpoint breakpoint);

        /**
         * Requests the TargetController to remove a breakpoint from the target.
         *
         * @param breakpoint
         */
        void removeBreakpointOnTarget(TargetBreakpoint breakpoint);

    public:
        DebugServer(EventManager& eventManager) : eventManager(eventManager) {};

        void setApplicationConfig(const ApplicationConfig& applicationConfig) {
            this->applicationConfig = applicationConfig;
        }

        void setEnvironmentConfig(const EnvironmentConfig& environmentConfig) {
            this->environmentConfig = environmentConfig;
        }

        void setDebugServerConfig(const DebugServerConfig& debugServerConfig) {
            this->debugServerConfig = debugServerConfig;
        }

        /**
         * Entry point for the DebugServer. This must called from a dedicated thread.
         */
        void run();

        virtual std::string getName() const = 0;

        virtual ~DebugServer() = default;
    };
}
