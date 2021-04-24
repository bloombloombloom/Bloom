#include <variant>
#include <cstdint>

#include "DebugServer.hpp"
#include "src/Exceptions/InvalidConfig.hpp"
#include "src/Logger/Logger.hpp"

using namespace Bloom::DebugServers;

void DebugServer::run() {
    try {
        this->startup();

        Logger::info("DebugServer ready");

        while (this->getState() == ThreadState::READY) {
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
    this->interruptEventNotifier->init();
    this->eventListener->setInterruptEventNotifier(this->interruptEventNotifier);

    // Register event handlers
    this->eventListener->registerCallbackForEventType<Events::ShutdownDebugServer>(
        std::bind(&DebugServer::onShutdownDebugServerEvent, this, std::placeholders::_1)
    );

    this->init();
    this->setStateAndEmitEvent(ThreadState::READY);
}

void DebugServer::shutdown() {
    if (this->getState() == ThreadState::STOPPED || this->getState() == ThreadState::SHUTDOWN_INITIATED) {
        return;
    }

    this->setState(ThreadState::SHUTDOWN_INITIATED);
    Logger::info("Shutting down DebugServer");
    this->close();
    this->interruptEventNotifier->close();
    this->setStateAndEmitEvent(ThreadState::STOPPED);
}

void DebugServer::onShutdownDebugServerEvent(EventPointer<Events::ShutdownDebugServer> event) {
    this->shutdown();
}