#include "DebugServerComponent.hpp"

#include "src/EventManager/EventManager.hpp"

// Debug server implementations
#include "Gdb/AvrGdb/AvrGdbRsp.hpp"
#include "Gdb/RiscVGdb/RiscVGdbRsp.hpp"

#include "src/Exceptions/InvalidConfig.hpp"
#include "src/Logger/Logger.hpp"

namespace DebugServer
{
    using namespace Events;

    DebugServerComponent::DebugServerComponent(const DebugServerConfig& debugServerConfig)
        : debugServerConfig(debugServerConfig)
        , targetDescriptor((Services::TargetControllerService()).getTargetDescriptor())
    {}

    void DebugServerComponent::run() {
        try {
            this->startup();

            Logger::info("DebugServer ready");

            while (this->getThreadState() == ThreadState::READY) {
                this->server->run();
                this->eventListener->dispatchCurrentEvents();
            }

        } catch (const std::exception& exception) {
            Logger::error("DebugServer fatal error: " + std::string{exception.what()});
        }

        this->shutdown();
    }

    std::map<
        std::string,
        std::function<std::unique_ptr<ServerInterface>()>
    > DebugServerComponent::getAvailableServersByName() {
        return std::map<std::string, std::function<std::unique_ptr<ServerInterface>()>> {
            {
                "avr-gdb-rsp",
                [this] () -> std::unique_ptr<ServerInterface> {
                    if (this->targetDescriptor.family != Targets::TargetFamily::AVR_8) {
                        throw Exceptions::Exception{"The AVR GDB RSP server is only compatible with AVR8 targets."};
                    }

                    return std::make_unique<DebugServer::Gdb::AvrGdb::AvrGdbRsp>(
                        this->debugServerConfig,
                        this->targetDescriptor,
                        *(this->eventListener.get()),
                        this->interruptEventNotifier
                    );
                }
            },
            {
                "riscv-gdb-rsp",
                [this] () -> std::unique_ptr<ServerInterface> {
                    if (this->targetDescriptor.family != Targets::TargetFamily::RISC_V) {
                        throw Exceptions::Exception{"The RISC-V GDB RSP server is only compatible with RISC-V targets."};
                    }

                    return std::make_unique<DebugServer::Gdb::RiscVGdb::RiscVGdbRsp>(
                        this->debugServerConfig,
                        this->targetDescriptor,
                        *(this->eventListener.get()),
                        this->interruptEventNotifier
                    );
                }
            },
        };
    }

    void DebugServerComponent::startup() {
        this->setName("DS");
        Logger::info("Starting DebugServer");
        this->blockAllSignals();

        EventManager::registerListener(this->eventListener);
        this->eventListener->setInterruptEventNotifier(&this->interruptEventNotifier);

        // Register event handlers
        this->eventListener->registerCallbackForEventType<Events::ShutdownDebugServer>(
            std::bind(&DebugServerComponent::onShutdownDebugServerEvent, this, std::placeholders::_1)
        );

        static const auto availableServersByName = this->getAvailableServersByName();
        const auto selectedServerIt = availableServersByName.find(this->debugServerConfig.name);

        if (selectedServerIt == availableServersByName.end()) {
            throw Exceptions::InvalidConfig{"DebugServer \"" + this->debugServerConfig.name + "\" not found."};
        }

        this->server = selectedServerIt->second();
        Logger::info("Selected DebugServer: " + this->server->getName());

        this->server->init();
        this->setThreadStateAndEmitEvent(ThreadState::READY);
    }

    void DebugServerComponent::shutdown() {
        const auto threadState = this->getThreadState();
        if (threadState == ThreadState::STOPPED || threadState == ThreadState::SHUTDOWN_INITIATED) {
            return;
        }

        this->threadState = ThreadState::SHUTDOWN_INITIATED;
        Logger::info("Shutting down DebugServer");

        if (this->server) {
            this->server->close();
        }

        this->setThreadStateAndEmitEvent(ThreadState::STOPPED);

        this->eventListener->setInterruptEventNotifier(nullptr);
        EventManager::deregisterListener(this->eventListener->getId());
    }

    void DebugServerComponent::setThreadStateAndEmitEvent(ThreadState state) {
        this->threadState = state;
        EventManager::triggerEvent(
            std::make_shared<Events::DebugServerThreadStateChanged>(state)
        );
    }

    void DebugServerComponent::onShutdownDebugServerEvent(const Events::ShutdownDebugServer&) {
        this->shutdown();
    }
}
