#pragma once

#include <csignal>

#include "src/Helpers/Thread.hpp"
#include "src/EventManager/EventManager.hpp"
#include "src/Helpers/SyncSafe.hpp"

namespace Bloom
{
    class SignalHandler: public Thread
    {
    public:
        SignalHandler() = default;

        /**
         * Entry point for SignalHandler thread.
         */
        void run();

        /**
         * Triggers the shutdown of the SignalHandler thread.
         */
        void triggerShutdown() {
            this->setThreadState(ThreadState::SHUTDOWN_INITIATED);
        };

    private:
        /**
         * Mapping of signal numbers to handler functions.
         */
        std::map<int, std::function<void()>> handlersBySignalNum;

        /**
         * We keep record of the number of shutdown signals received. See definition of triggerApplicationShutdown()
         * for more on this.
         */
        int shutdownSignalsReceived = 0;

        /**
         * Initiates the SignalHandler thread.
         */
        void startup();

        /**
         * Fetches all signals currently of interest to the application.
         *
         * Currently this just returns a set of all signals (using sigfillset())
         *
         * @return
         */
        [[nodiscard]] ::sigset_t getRegisteredSignalSet() const;

        /**
         * Handler for SIGINT, SIGTERM, etc signals.
         *
         * Will trigger a ShutdownApplication event to kick-off a clean shutdown or it will terminate the
         * program immediately if numerous SIGINT signals have been received.
         */
        void triggerApplicationShutdown();
    };
}
