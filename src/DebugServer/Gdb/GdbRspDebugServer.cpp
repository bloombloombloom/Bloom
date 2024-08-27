#include "GdbRspDebugServer.hpp"

#include <sys/socket.h>
#include <unistd.h>

#include "src/EventManager/EventManager.hpp"
#include "src/Logger/Logger.hpp"

#include "Exceptions/ClientDisconnected.hpp"
#include "Exceptions/ClientNotSupported.hpp"
#include "Exceptions/ClientCommunicationError.hpp"
#include "Exceptions/DebugSessionInitialisationFailure.hpp"
#include "Exceptions/DebugServerInterrupted.hpp"

#include "src/Exceptions/Exception.hpp"
#include "src/Exceptions/InvalidConfig.hpp"

// Command packets
#include "CommandPackets/CommandPacket.hpp"
#include "CommandPackets/SupportedFeaturesQuery.hpp"
#include "CommandPackets/InterruptExecution.hpp"
#include "CommandPackets/ContinueExecution.hpp"
#include "CommandPackets/StepExecution.hpp"
#include "CommandPackets/SetBreakpoint.hpp"
#include "CommandPackets/RemoveBreakpoint.hpp"
#include "CommandPackets/Monitor.hpp"
#include "CommandPackets/ResetTarget.hpp"
#include "CommandPackets/HelpMonitorInfo.hpp"
#include "CommandPackets/BloomVersion.hpp"
#include "CommandPackets/BloomVersionMachine.hpp"
#include "CommandPackets/Detach.hpp"
#include "CommandPackets/ListRegistersMonitor.hpp"
#include "CommandPackets/ReadRegistersMonitor.hpp"
#include "CommandPackets/WriteRegisterMonitor.hpp"

#ifndef EXCLUDE_INSIGHT
#include "CommandPackets/ActivateInsight.hpp"
#endif

// Response packets
#include "ResponsePackets/TargetStopped.hpp"

#include "src/Services/ProcessService.hpp"
#include "src/Services/StringService.hpp"

namespace DebugServer::Gdb
{
    using namespace Exceptions;
    using namespace ::Exceptions;

    using CommandPackets::CommandPacket;

    GdbRspDebugServer::GdbRspDebugServer(
        const DebugServerConfig& debugServerConfig,
        const Targets::TargetDescriptor& targetDescriptor,
        EventListener& eventListener,
        EventFdNotifier& eventNotifier
    )
        : debugServerConfig(GdbDebugServerConfig(debugServerConfig))
        , targetDescriptor(targetDescriptor)
        , eventListener(eventListener)
        , interruptEventNotifier(eventNotifier)
    {}

    void GdbRspDebugServer::init() {
        this->socketAddress.sin_family = AF_INET;
        this->socketAddress.sin_port = htons(this->debugServerConfig.listeningPortNumber);

        if (
            ::inet_pton(
                AF_INET,
                this->debugServerConfig.listeningAddress.c_str(),
                &(this->socketAddress.sin_addr)
            ) == 0
        ) {
            // Invalid IP address
            throw InvalidConfig{
                "Invalid IP address provided in config file: (\"" + this->debugServerConfig.listeningAddress + "\")"
            };
        }

        auto socketFileDescriptor = int{0};
        if ((socketFileDescriptor = ::socket(AF_INET, SOCK_STREAM, 0)) == 0) {
            throw Exception{"Failed to create socket file descriptor."};
        }

        const auto enableReuseAddressSocketOption = int{1};
        if (
            ::setsockopt(
                socketFileDescriptor,
                SOL_SOCKET,
                SO_REUSEADDR,
                &(enableReuseAddressSocketOption),
                sizeof(enableReuseAddressSocketOption)
            ) < 0
        ) {
            Logger::error("Failed to set socket SO_REUSEADDR option.");
        }

        if (
            ::bind(
                socketFileDescriptor,
                reinterpret_cast<const sockaddr*>(&(this->socketAddress)),
                sizeof(this->socketAddress)
            ) < 0
        ) {
            throw Exception{
                "Failed to bind address. The selected port number ("
                    + std::to_string(this->debugServerConfig.listeningPortNumber) + ") may be in use."
            };
        }

        this->serverSocketFileDescriptor = socketFileDescriptor;

        this->epollInstance.addEntry(
            this->serverSocketFileDescriptor.value(),
            static_cast<std::uint16_t>(::EPOLL_EVENTS::EPOLLIN)
        );

        this->epollInstance.addEntry(
            this->interruptEventNotifier.getFileDescriptor(),
            static_cast<std::uint16_t>(::EPOLL_EVENTS::EPOLLIN)
        );

        Logger::info("GDB RSP address: " + this->debugServerConfig.listeningAddress);
        Logger::info("GDB RSP port: " + std::to_string(this->debugServerConfig.listeningPortNumber));

        this->eventListener.registerCallbackForEventType<Events::TargetStateChanged>(
            std::bind(&GdbRspDebugServer::onTargetStateChanged, this, std::placeholders::_1)
        );

        if (Services::ProcessService::isManagedByClion()) {
            Logger::warning(
                "Bloom's process is being managed by CLion - Bloom will automatically shutdown upon detaching from GDB."
            );
        }
    }

    void GdbRspDebugServer::close() {
        this->endDebugSession();

        if (this->serverSocketFileDescriptor.has_value()) {
            ::close(this->serverSocketFileDescriptor.value());
        }
    }

    void GdbRspDebugServer::run() {
        try {
            if (this->getActiveDebugSession() == nullptr) {
                Logger::info("Waiting for GDB RSP connection");

                auto connection = this->waitForConnection();
                Logger::info("Accepted GDP RSP connection from " + connection.getIpAddress());

                this->startDebugSession(std::move(connection));

                this->targetControllerService.stopTargetExecution();
                this->targetControllerService.resetTarget();
            }

            const auto commandPacket = this->waitForCommandPacket();
            if (commandPacket) {
                commandPacket->handle(
                    *(this->getActiveDebugSession()),
                    this->getGdbTargetDescriptor(),
                    this->targetDescriptor,
                    this->targetControllerService
                );
            }

        } catch (const ClientDisconnected&) {
            Logger::info("GDB RSP client disconnected");
            this->endDebugSession();
            return;

        } catch (const ClientCommunicationError& exception) {
            Logger::error(
                "GDB RSP client communication error - " + exception.getMessage() + " - closing connection"
            );
            this->endDebugSession();
            return;

        } catch (const ClientNotSupported& exception) {
            Logger::error("Invalid GDB RSP client - " + exception.getMessage() + " - closing connection");
            this->endDebugSession();
            return;

        } catch (const DebugSessionInitialisationFailure& exception) {
            Logger::warning("GDB debug session initialisation failure - " + exception.getMessage());
            this->endDebugSession();
            return;

        } catch (const DebugServerInterrupted&) {
            // Server was interrupted by an event
            Logger::debug("GDB RSP interrupted");
            return;
        }
    }

    Connection GdbRspDebugServer::waitForConnection() {
        if (::listen(this->serverSocketFileDescriptor.value(), 3) != 0) {
            throw Exception{"Failed to listen on server socket"};
        }

        const auto eventFileDescriptor = this->epollInstance.waitForEvent();
        if (
            !eventFileDescriptor.has_value()
            || *eventFileDescriptor == this->interruptEventNotifier.getFileDescriptor()
        ) {
            this->interruptEventNotifier.clear();
            throw DebugServerInterrupted{};
        }

        return {this->serverSocketFileDescriptor.value(), this->interruptEventNotifier};
    }

    std::unique_ptr<CommandPacket> GdbRspDebugServer::waitForCommandPacket() {
        auto* debugSession = this->getActiveDebugSession();
        const auto rawPackets = debugSession->connection.readRawPackets();
        assert(!rawPackets.empty());

        if (rawPackets.size() > 1) {
            const auto& firstRawPacket = rawPackets.front();

            if (firstRawPacket.size() == 5 && firstRawPacket[1] == 0x03) {
                // Interrupt packet that came in too quickly before another packet
                debugSession->pendingInterrupt = true;

            } else {
                Logger::warning("Multiple packets received from GDB - only the most recent will be processed");
            }
        }

        return this->resolveCommandPacket(rawPackets.back());
    }

    std::unique_ptr<CommandPacket> GdbRspDebugServer::resolveCommandPacket(const RawPacket& rawPacket) {
        if (rawPacket.size() < 2) {
            throw Exception{"Invalid raw packet - no data"};
        }

        if (rawPacket.size() == 5 && rawPacket[1] == 0x03) {
            // Interrupt request
            return std::make_unique<CommandPackets::InterruptExecution>(rawPacket);
        }

        if (rawPacket[1] == 'c') {
            return std::make_unique<CommandPackets::ContinueExecution>(rawPacket);
        }

        if (rawPacket[1] == 's') {
            return std::make_unique<CommandPackets::StepExecution>(rawPacket);
        }

        if (rawPacket[1] == 'Z') {
            return std::make_unique<CommandPackets::SetBreakpoint>(rawPacket);
        }

        if (rawPacket[1] == 'z') {
            return std::make_unique<CommandPackets::RemoveBreakpoint>(rawPacket);
        }

        if (rawPacket[1] == 'D') {
            return std::make_unique<CommandPackets::Detach>(rawPacket);
        }

        const auto rawPacketString = std::string{rawPacket.begin() + 1, rawPacket.end()};

        /*
         * First byte of the raw packet will be 0x24 ('$'), so std::string::find() should return 1, not 0, when
         * looking for a command identifier string.
         */
        if (rawPacketString.find("qSupported") == 0) {
            return std::make_unique<CommandPackets::SupportedFeaturesQuery>(rawPacket);
        }

        if (rawPacketString.find("qRcmd") == 0) {
            // This is a monitor packet
            auto monitorCommand = std::make_unique<CommandPackets::Monitor>(rawPacket);

            if (monitorCommand->command == "help") {
                return std::make_unique<CommandPackets::HelpMonitorInfo>(std::move(*(monitorCommand.release())));
            }

            if (monitorCommand->command == "version") {
                return std::make_unique<CommandPackets::BloomVersion>(std::move(*(monitorCommand.release())));
            }

            if (monitorCommand->command == "version machine") {
                return std::make_unique<CommandPackets::BloomVersionMachine>(std::move(*(monitorCommand.release())));
            }

            if (monitorCommand->command == "reset") {
                return std::make_unique<CommandPackets::ResetTarget>(std::move(*(monitorCommand.release())));
            }

            if (monitorCommand->command.find("lr") == 0) {
                return std::make_unique<CommandPackets::ListRegistersMonitor>(std::move(*(monitorCommand.release())));
            }

            if (monitorCommand->command.find("rr") == 0) {
                return std::make_unique<CommandPackets::ReadRegistersMonitor>(std::move(*(monitorCommand.release())));
            }

            if (monitorCommand->command.find("wr") == 0) {
                return std::make_unique<CommandPackets::WriteRegisterMonitor>(std::move(*(monitorCommand.release())));
            }

#ifndef EXCLUDE_INSIGHT
            if (monitorCommand->command.find("insight") == 0) {
                return std::make_unique<CommandPackets::ActivateInsight>(std::move(*(monitorCommand.release())));
            }
#endif
            return monitorCommand;
        }

        return std::make_unique<CommandPacket>(rawPacket);
    }

    void GdbRspDebugServer::onTargetStateChanged(const Events::TargetStateChanged& event) {
        using Targets::TargetExecutionState;

        auto* debugSession = this->getActiveDebugSession();
        if (debugSession == nullptr) {
            return;
        }

        if (event.newState.executionState == event.previousState.executionState) {
            // Execution state hasn't changed. Probably just a mode change. Ignore...
            return;
        }

        const auto executionState = event.newState.executionState.load();

        try {
            if (executionState == TargetExecutionState::STOPPED && debugSession->waitingForBreak) {
                this->handleTargetStoppedGdbResponse(event.newState.programCounter.load().value());
                return;
            }

            if (executionState == TargetExecutionState::RUNNING || executionState == TargetExecutionState::STEPPING) {
                this->handleTargetResumedGdbResponse();
                return;
            }

        } catch (const ClientDisconnected&) {
            Logger::info("GDB RSP client disconnected");
            this->endDebugSession();
            return;

        } catch (const ClientCommunicationError& exception) {
            Logger::error(
                "GDB RSP client communication error - " + exception.getMessage() + " - closing connection"
            );
            this->endDebugSession();
            return;

        } catch (const DebugServerInterrupted&) {
            // Server was interrupted
            Logger::debug("GDB RSP interrupted");
            return;

        } catch (const Exception& exception) {
            Logger::error("Failed to handle target execution stopped event - " + exception.getMessage());
        }
    }

    void GdbRspDebugServer::handleTargetStoppedGdbResponse(Targets::TargetMemoryAddress programAddress) {
        auto* debugSession = this->getActiveDebugSession();

        if (debugSession->activeRangeSteppingSession.has_value()) {
            debugSession->terminateRangeSteppingSession(this->targetControllerService);
        }

        debugSession->connection.writePacket(ResponsePackets::TargetStopped{Signal::TRAP});
        debugSession->waitingForBreak = false;
    }

    void GdbRspDebugServer::handleTargetResumedGdbResponse() {
        auto* debugSession = this->getActiveDebugSession();

        if (debugSession->waitingForBreak && debugSession->pendingInterrupt) {
            Logger::info("Servicing pending interrupt");
            this->targetControllerService.stopTargetExecution();

            if (debugSession->activeRangeSteppingSession.has_value()) {
                debugSession->terminateRangeSteppingSession(this->targetControllerService);
            }

            debugSession->connection.writePacket(ResponsePackets::TargetStopped{Signal::INTERRUPTED});
            debugSession->pendingInterrupt = false;
            debugSession->waitingForBreak = false;
        }
    }
}
