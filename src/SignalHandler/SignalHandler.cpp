#include "SignalHandler.hpp"

#include <csignal>
#include <thread>

#include "src/Logger/Logger.hpp"
#include "src/Exceptions/Exception.hpp"

namespace Bloom
{
    void SignalHandler::run() {
        try {
            this->startup();
            const auto signalSet = this->getRegisteredSignalSet();
            int signalNumber = 0;

            Logger::debug("SignalHandler ready");
            while(Thread::getThreadState() == ThreadState::READY) {
                if (::sigwait(&signalSet, &signalNumber) == 0) {
                    Logger::debug("SIGNAL " + std::to_string(signalNumber) + " received");

                    const auto& handlerIt = this->handlersBySignalNum.find(signalNumber);
                    if (handlerIt != this->handlersBySignalNum.end()) {
                        // We have a registered handler for this signal.
                        handlerIt->second();
                    }
                }
            }

        } catch (std::exception& exception) {
            Logger::error("SignalHandler fatal error: " + std::string(exception.what()));
        }

        Logger::info("Shutting down SignalHandler");
        this->threadState = ThreadState::STOPPED;
    }

    void SignalHandler::startup() {
        this->setName("SH");
        this->threadState = ThreadState::STARTING;
        Logger::debug("Starting SignalHandler");
        // Block all signal interrupts
        auto signalSet = this->getRegisteredSignalSet();
        ::sigprocmask(SIG_SETMASK, &signalSet, NULL);

        // Register handlers
        this->handlersBySignalNum.insert(std::pair(
            SIGINT,
            std::bind(&SignalHandler::triggerApplicationShutdown, this)
        ));

        this->handlersBySignalNum.insert(std::pair(
            SIGTERM,
            std::bind(&SignalHandler::triggerApplicationShutdown, this)
        ));

        // It's possible that the SignalHandler has been instructed to shut down, before it could finish starting up.
        if (this->getThreadState() != ThreadState::SHUTDOWN_INITIATED) {
            this->threadState = ThreadState::READY;
        }
    }

    ::sigset_t SignalHandler::getRegisteredSignalSet() const {
        ::sigset_t set = {};
        if (::sigfillset(&set) == -1) {
            throw Exceptions::Exception("::sigfillset() failed - error number: " + std::to_string(errno));
        }

        return set;
    }

    void SignalHandler::triggerApplicationShutdown() {
        Logger::warning("Shutdown signal received");
        this->shutdownSignalsReceived++;

        if (this->shutdownSignalsReceived > 1) {
            // User has likely run out of patience
            Logger::warning("Aborting immediately");
            std::exit(EXIT_FAILURE);
        }

        Logger::info("Attempting clean shutdown");
        EventManager::triggerEvent(std::make_shared<Events::ShutdownApplication>());
    }
}
