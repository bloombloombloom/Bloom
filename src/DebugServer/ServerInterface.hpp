#pragma once

#include <string>

namespace Bloom::DebugServer
{
    /**
     * Every debug server must implement this interface.
     *
     * See documentation in src/DebugServer/README.md for more.
     */
    class ServerInterface
    {
    public:
        virtual ~ServerInterface() = default;

        /**
         * Should return the name of the server.
         *
         * @return
         */
        [[nodiscard]] virtual std::string getName() const = 0;

        /**
         * Called on startup of the DebugServerComponent. The server should implement any initialisation work here.
         */
        virtual void init() = 0;

        /**
         * Called repeatedly in an infinite loop when the DebugServerComponent is running. The server should serve
         * from here.
         *
         * For servicing DebugServer events, the implementation should either service them here or return from here
         * upon an event being triggered. Returning from this function will allow DebugServerComponent::run() to
         * process any pending events. See the DebugServer documentation in src/DebugServer/README.md for more.
         */
        virtual void run() = 0;

        /**
         * Called on shutdown of the DebugServerComponent.
         */
        virtual void close() = 0;
    };
}
