#pragma once

#include <cstdint>
#include <map>
#include <functional>
#include <memory>

#include "src/TargetController/TargetControllerConsole.hpp"
#include "src/EventManager/Events/Events.hpp"
#include "src/EventManager/EventManager.hpp"
#include "src/Exceptions/DebugServerInterrupted.hpp"
#include "src/ProjectConfig.hpp"
#include "src/Helpers/Thread.hpp"
#include "src/Targets/TargetDescriptor.hpp"
#include "src/Targets/TargetRegister.hpp"
#include "src/Targets/TargetBreakpoint.hpp"

#include "ServerInterface.hpp"

namespace Bloom::DebugServers
{
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
    public:
        explicit DebugServer(const DebugServerConfig& debugServerConfig);

        /**
         * Entry point for the DebugServer. This must called from a dedicated thread.
         */
        void run();

    protected:
        EventListenerPointer eventListener = std::make_shared<EventListener>("DebugServerEventListener");

        DebugServerConfig debugServerConfig;

        TargetControllerConsole targetControllerConsole = TargetControllerConsole(*(this->eventListener));

        /**
         * Enables the interruption of any blocking file IO.
         */
        EventNotifier interruptEventNotifier = EventNotifier();

    private:
        std::unique_ptr<ServerInterface> server = nullptr;

        std::map<std::string, std::function<std::unique_ptr<ServerInterface>()>> getAvailableServersByName();

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
        void setThreadStateAndEmitEvent(ThreadState state) {
            Thread::setThreadState(state);
            EventManager::triggerEvent(
                std::make_shared<Events::DebugServerThreadStateChanged>(state)
            );
        }

        /**
         * Handles a shutdown request.
         *
         * @param event
         */
        void onShutdownDebugServerEvent(const Events::ShutdownDebugServer& event);
    };
}
