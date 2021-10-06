#include "SignalHandler.hpp"

#include <csignal>
#include <thread>

#include "src/Logger/Logger.hpp"
#include "src/Exceptions/Exception.hpp"

using namespace Bloom;

void SignalHandler::run() {
    try {
        this->startup();
        auto signalSet = this->getRegisteredSignalSet();
        int signalNumber = 0;

        Logger::debug("SignalHandler ready");
        while(Thread::getThreadState() == ThreadState::READY) {
            if (sigwait(&signalSet, &signalNumber) == 0) {
                Logger::debug("SIGNAL " + std::to_string(signalNumber) + " received");
                if (this->handlersMappedBySignalNum.contains(signalNumber)) {
                    // We have a registered handler for this signal.
                    this->handlersMappedBySignalNum.at(signalNumber)();
                }
            }
        }

    } catch (std::exception& exception) {
        Logger::error("SignalHandler fatal error: " + std::string(exception.what()));
    }

    Logger::info("Shutting down SignalHandler");
    Thread::setThreadState(ThreadState::STOPPED);
}

void SignalHandler::startup() {
    this->setName("SH");
    Thread::setThreadState(ThreadState::STARTING);
    Logger::debug("Starting SignalHandler");
    // Block all signal interrupts
    auto signalSet = this->getRegisteredSignalSet();
    sigprocmask(SIG_SETMASK, &signalSet, NULL);

    // Register handlers here
    this->handlersMappedBySignalNum.insert(std::pair(
        SIGINT,
        std::bind(&SignalHandler::triggerApplicationShutdown, this)
    ));

    this->handlersMappedBySignalNum.insert(std::pair(
        SIGTERM,
        std::bind(&SignalHandler::triggerApplicationShutdown, this)
    ));

    // It's possible that the SignalHandler has been instructed to shutdown, before it could finish starting up.
    if (this->getThreadState() != ThreadState::SHUTDOWN_INITIATED) {
        Thread::setThreadState(ThreadState::READY);
    }
}

sigset_t SignalHandler::getRegisteredSignalSet() const {
    sigset_t set = {};
    if (sigfillset(&set) == -1) {
        throw Exceptions::Exception("sigfillset() failed - error number: " + std::to_string(errno));
    }

    return set;
}

void SignalHandler::triggerApplicationShutdown() {
    Logger::warning("Shutdown signal received");
    this->shutdownSignalsReceived++;

    if (this->shutdownSignalsReceived > 1) {
        // User has likely run out of patience
        Logger::warning("Aborting immediately");
        exit(EXIT_FAILURE);
    }

    Logger::info("Attempting clean shutdown");
    this->eventManager.triggerEvent(std::make_shared<Events::ShutdownApplication>());
}
