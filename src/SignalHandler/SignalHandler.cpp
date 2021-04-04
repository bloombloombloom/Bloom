#include <iostream>
#include <csignal>
#include <QtCore>
#include <experimental/filesystem>
#include <thread>
#include <src/Exceptions/Exception.hpp>
#include <src/EventManager/Events/ShutdownTargetController.hpp>

#include "src/Logger/Logger.hpp"
#include "SignalHandler.hpp"

using namespace Bloom;

void SignalHandler::run() {
    try {
        this->startup();
        auto signalSet = this->getRegisteredSignalSet();
        int signalNumber = 0;

        Logger::debug("SignalHandler ready");
        while(Thread::getState() == ThreadState::READY) {
            if (sigwait(&signalSet, &signalNumber) == 0) {
                Logger::debug("SIGNAL " + std::to_string(signalNumber) + " received");
                if (this->handlersMappedBySignalNum.find(signalNumber) != this->handlersMappedBySignalNum.end()) {
                    // We have a registered handler for this signal.
                    this->handlersMappedBySignalNum.find(signalNumber)->second();
                }
            }
        }

    } catch (std::exception& exception) {
        Logger::error("SignalHandler fatal error: " + std::string(exception.what()));
    }

    Logger::debug("SignalHandler shutting down");
    Thread::setState(ThreadState::STOPPED);
}

void SignalHandler::startup() {
    this->setName("SH");
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

    Thread::setState(ThreadState::READY);
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
