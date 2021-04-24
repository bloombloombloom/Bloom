#pragma once

#include <map>
#include <functional>
#include <cstdint>

#include "src/TargetController/TargetControllerConsole.hpp"
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

        TargetControllerConsole targetControllerConsole = TargetControllerConsole(
            this->eventManager,
            *(this->eventListener)
        );

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

    public:
        DebugServer(EventManager& eventManager): eventManager(eventManager) {};

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
