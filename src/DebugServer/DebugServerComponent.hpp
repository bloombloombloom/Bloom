#pragma once

#include <cstdint>
#include <map>
#include <functional>
#include <memory>

#include "src/Helpers/Thread.hpp"
#include "src/ProjectConfig.hpp"
#include "src/Helpers/EventFdNotifier.hpp"
#include "src/EventManager/EventListener.hpp"
#include "src/EventManager/Events/Events.hpp"

#include "ServerInterface.hpp"

namespace Bloom::DebugServer
{
    /**
     * The DebugServer exposes the connected target to third-party debugging software such as IDEs.
     *
     * See documentation in src/DebugServer/README.md for more.
     */
    class DebugServerComponent: public Thread
    {
    public:
        explicit DebugServerComponent(const DebugServerConfig& debugServerConfig);

        /**
         * Entry point for the DebugServer. This must called from a dedicated thread.
         *
         * See Application::startDebugServer() for more.
         */
        void run();

    private:
        /**
         * The DebugServer's event listener.
         *
         * If a server implementation needs access to events, they should use this event listener as opposed to
         * creating their own. See the AVR GDB server implementation for an example - there, we inject this event
         * listener via AvrGdbRsp's constructor (see DebugServerComponent::getAvailableServersByName()).
         */
        EventListenerPointer eventListener = std::make_shared<EventListener>("DebugServerEventListener");

        /**
         * Project configuration for the debug server (extracted from the user project's bloom.yaml).
         */
        DebugServerConfig debugServerConfig;

        /**
         * This EventFdNotifier is injected into this->eventListener. It can be used by server implementations to
         * interrupt blocking I/O calls upon an event being triggered. For more, see the "Servicing events" section in
         * the DebugServer documentation (src/DebugServer/README.md).
         */
        EventFdNotifier interruptEventNotifier = EventFdNotifier();

        /**
         * An instance to the selected server implementation. See DebugServerComponent::startup() for more.
         */
        std::unique_ptr<ServerInterface> server = nullptr;

        /**
         * Returns a mapping of server configuration names to lambdas/std::functions.
         *
         * The server configuration name is what the user will use in their project configuration (bloom.yaml), when
         * selecting a debug server. It *must* be lower-case.
         *
         * The lambda should return an instance of the server implementation.
         *
         * @return
         */
        std::map<std::string, std::function<std::unique_ptr<ServerInterface>()>> getAvailableServersByName();

        /**
         * Prepares the debug server thread and then calls this->server->init().
         */
        void startup();

        /**
         * Calls this->server->close() and updates the thread state.
         */
        void shutdown();

        /**
         * Updates the state of the DebugServer and emits a state changed event.
         *
         * @param state
         * @param emitEvent
         */
        void setThreadStateAndEmitEvent(ThreadState state);

        /**
         * Handles a shutdown request.
         *
         * @param event
         */
        void onShutdownDebugServerEvent(const Events::ShutdownDebugServer& event);
    };
}
