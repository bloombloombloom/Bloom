#pragma once

#include <string>

namespace Bloom::DebugServer
{
    class ServerInterface
    {
    public:
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
         * This function should return when any blocking operation is interrupted via an EventNotifier instance.
         */
        virtual void run() = 0;

        /**
         * Called on shutdown of the DebugServerComponent.
         */
        virtual void close() = 0;
    };
}
