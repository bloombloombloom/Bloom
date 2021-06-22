#include <variant>
#include <cstdint>

#include "DebugServer.hpp"
#include "src/Exceptions/InvalidConfig.hpp"
#include "src/Logger/Logger.hpp"

using namespace Bloom::DebugServers;
using namespace Bloom::Events;

void DebugServer::run() {
    try {
        this->startup();

        Logger::info("DebugServer ready");

        while (this->getThreadState() == ThreadState::READY) {
            this->serve();
            this->eventListener->dispatchCurrentEvents();
        }
    } catch (const std::exception& exception) {
        Logger::error("DebugServer fatal error: " + std::string(exception.what()));
    }

    this->shutdown();
}

void DebugServer::startup() {
    this->setName("DS");
    Logger::info("Starting DebugServer");

    this->eventManager.registerListener(this->eventListener);

    this->interruptEventNotifier = std::make_shared<EventNotifier>();
    this->eventListener->setInterruptEventNotifier(this->interruptEventNotifier);

    // Register event handlers
    this->eventListener->registerCallbackForEventType<Events::ShutdownDebugServer>(
        std::bind(&DebugServer::onShutdownDebugServerEvent, this, std::placeholders::_1)
    );

    this->init();
    this->setThreadStateAndEmitEvent(ThreadState::READY);
}

void DebugServer::shutdown() {
    if (this->getThreadState() == ThreadState::STOPPED || this->getThreadState() == ThreadState::SHUTDOWN_INITIATED) {
        return;
    }

    this->setThreadState(ThreadState::SHUTDOWN_INITIATED);
    Logger::info("Shutting down DebugServer");
    this->close();
    this->setThreadStateAndEmitEvent(ThreadState::STOPPED);
    this->eventManager.deregisterListener(this->eventListener->getId());
}

void DebugServer::onShutdownDebugServerEvent(EventRef<Events::ShutdownDebugServer> event) {
    this->shutdown();
}
