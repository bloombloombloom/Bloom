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
#include "CommandPackets/GenerateSvd.hpp"
#include "CommandPackets/Detach.hpp"
#include "CommandPackets/EepromFill.hpp"

#ifndef EXCLUDE_INSIGHT
#include "CommandPackets/ActivateInsight.hpp"
#endif

// Response packets
#include "ResponsePackets/TargetStopped.hpp"

#include "src/Services/ProcessService.hpp"

namespace DebugServer::Gdb
{
    using namespace Exceptions;
    using namespace ::Exceptions;

    using CommandPackets::CommandPacket;

    GdbRspDebugServer::GdbRspDebugServer(
        const DebugServerConfig& debugServerConfig,
        EventListener& eventListener,
        EventFdNotifier& eventNotifier
    )
        : debugServerConfig(GdbDebugServerConfig(debugServerConfig))
        , eventListener(eventListener)
        , interruptEventNotifier(eventNotifier)
    {}

    void GdbRspDebugServer::init() {
        this->socketAddress.sin_family = AF_INET;
        this->socketAddress.sin_port = htons(this->debugServerConfig.listeningPortNumber);

        if (::inet_pton(
                AF_INET,
                this->debugServerConfig.listeningAddress.c_str(),
                &(this->socketAddress.sin_addr)
            ) == 0
        ) {
            // Invalid IP address
            throw InvalidConfig(
                "Invalid IP address provided in config file: (\"" + this->debugServerConfig.listeningAddress
                    + "\")"
            );
        }

        int socketFileDescriptor = 0;

        if ((socketFileDescriptor = ::socket(AF_INET, SOCK_STREAM, 0)) == 0) {
            throw Exception("Failed to create socket file descriptor.");
        }

        const auto enableReuseAddressSocketOption = 1;

        if (::setsockopt(
                socketFileDescriptor,
                SOL_SOCKET,
                SO_REUSEADDR,
                &(enableReuseAddressSocketOption),
                sizeof(enableReuseAddressSocketOption)
            ) < 0
        ) {
            Logger::error("Failed to set socket SO_REUSEADDR option.");
        }

        if (::bind(
                socketFileDescriptor,
                reinterpret_cast<const sockaddr*>(&(this->socketAddress)),
                sizeof(this->socketAddress)
            ) < 0
        ) {
            throw Exception("Failed to bind address. The selected port number ("
                + std::to_string(this->debugServerConfig.listeningPortNumber) + ") may be in use.");
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

        this->eventListener.registerCallbackForEventType<Events::TargetExecutionStopped>(
            std::bind(&GdbRspDebugServer::onTargetExecutionStopped, this, std::placeholders::_1)
        );

        this->eventListener.registerCallbackForEventType<Events::TargetExecutionResumed>(
            std::bind(&GdbRspDebugServer::onTargetExecutionResumed, this, std::placeholders::_1)
        );

        if (Services::ProcessService::isManagedByClion()) {
            Logger::warning(
                "Bloom's process is being managed by CLion - Bloom will automatically shutdown upon detaching from GDB."
            );
        }
    }

    void GdbRspDebugServer::close() {
        this->activeDebugSession.reset();

        if (this->serverSocketFileDescriptor.has_value()) {
            ::close(this->serverSocketFileDescriptor.value());
        }
    }

    void GdbRspDebugServer::run() {
        try {
            if (!this->activeDebugSession.has_value()) {
                Logger::info("Waiting for GDB RSP connection");

                auto connection = this->waitForConnection();

                Logger::info("Accepted GDP RSP connection from " + connection.getIpAddress());

                this->activeDebugSession.emplace(
                    std::move(connection),
                    this->getSupportedFeatures(),
                    this->getGdbTargetDescriptor(),
                    this->debugServerConfig
                );

                this->targetControllerService.stopTargetExecution();
                this->targetControllerService.resetTarget();
            }

            const auto commandPacket = this->waitForCommandPacket();

            if (commandPacket) {
                commandPacket->handle(this->activeDebugSession.value(), this->targetControllerService);
            }

        } catch (const ClientDisconnected&) {
            Logger::info("GDB RSP client disconnected");
            this->activeDebugSession.reset();
            return;

        } catch (const ClientCommunicationError& exception) {
            Logger::error(
                "GDB RSP client communication error - " + exception.getMessage() + " - closing connection"
            );
            this->activeDebugSession.reset();
            return;

        } catch (const ClientNotSupported& exception) {
            Logger::error("Invalid GDB RSP client - " + exception.getMessage() + " - closing connection");
            this->activeDebugSession.reset();
            return;

        } catch (const DebugSessionInitialisationFailure& exception) {
            Logger::warning("GDB debug session initialisation failure - " + exception.getMessage());
            this->activeDebugSession.reset();
            return;

        } catch (const DebugServerInterrupted&) {
            // Server was interrupted by an event
            Logger::debug("GDB RSP interrupted");
            return;
        }
    }

    Connection GdbRspDebugServer::waitForConnection() {
        if (::listen(this->serverSocketFileDescriptor.value(), 3) != 0) {
            throw Exception("Failed to listen on server socket");
        }

        const auto eventFileDescriptor = this->epollInstance.waitForEvent();

        if (
            !eventFileDescriptor.has_value()
            || eventFileDescriptor.value() == this->interruptEventNotifier.getFileDescriptor()
        ) {
            this->interruptEventNotifier.clear();
            throw DebugServerInterrupted();
        }

        return Connection(
            this->serverSocketFileDescriptor.value(),
            this->interruptEventNotifier
        );
    }

    std::unique_ptr<CommandPacket> GdbRspDebugServer::waitForCommandPacket() {
        const auto rawPackets = this->activeDebugSession->connection.readRawPackets();

        if (rawPackets.size() > 1) {
            const auto& firstRawPacket = rawPackets.front();

            if (firstRawPacket.size() == 5 && firstRawPacket[1] == 0x03) {
                // Interrupt packet that came in too quickly before another packet
                this->activeDebugSession->pendingInterrupt = true;
            }

            // We only process the last packet - any others will probably be duplicates from an impatient client.
            Logger::warning("Multiple packets received from GDB - only the most recent will be processed");
        }

        return this->resolveCommandPacket(rawPackets.back());
    }

    std::unique_ptr<CommandPacket> GdbRspDebugServer::resolveCommandPacket(const RawPacket& rawPacket) {
        if (rawPacket.size() == 5 && rawPacket[1] == 0x03) {
            // Interrupt request
            return std::make_unique<CommandPackets::InterruptExecution>(rawPacket);
        }

        if (rawPacket[1] == 'D') {
            return std::make_unique<CommandPackets::Detach>(rawPacket);
        }

        const auto rawPacketString = std::string(rawPacket.begin(), rawPacket.end());

        if (rawPacketString.size() >= 2) {
            /*
             * First byte of the raw packet will be 0x24 ('$'), so std::string::find() should return 1, not 0, when
             * looking for a command identifier string.
             */
            if (rawPacketString.find("qSupported") == 1) {
                return std::make_unique<CommandPackets::SupportedFeaturesQuery>(rawPacket);
            }

            if (rawPacketString[1] == 'c') {
                return std::make_unique<CommandPackets::ContinueExecution>(rawPacket);
            }

            if (rawPacketString[1] == 's') {
                return std::make_unique<CommandPackets::StepExecution>(rawPacket);
            }

            if (rawPacketString[1] == 'Z') {
                return std::make_unique<CommandPackets::SetBreakpoint>(rawPacket);
            }

            if (rawPacketString[1] == 'z') {
                return std::make_unique<CommandPackets::RemoveBreakpoint>(rawPacket);
            }

            if (rawPacketString.find("qRcmd") == 1) {
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

                if (monitorCommand->command.find("svd") == 0) {
                    return std::make_unique<CommandPackets::GenerateSvd>(std::move(*(monitorCommand.release())));
                }

                if (monitorCommand->command.find("eeprom fill") == 0) {
                    return std::make_unique<CommandPackets::EepromFill>(std::move(*(monitorCommand.release())));
                }
#ifndef EXCLUDE_INSIGHT
                if (monitorCommand->command.find("insight") == 0) {
                    return std::make_unique<CommandPackets::ActivateInsight>(std::move(*(monitorCommand.release())));
                }
#endif
                return monitorCommand;
            }
        }

        return std::make_unique<CommandPacket>(rawPacket);
    }

    std::set<std::pair<Feature, std::optional<std::string>>> GdbRspDebugServer::getSupportedFeatures() {
        return {
            {Feature::SOFTWARE_BREAKPOINTS, std::nullopt},
        };
    }

    void GdbRspDebugServer::onTargetExecutionStopped(const Events::TargetExecutionStopped&) {
        try {
            if (this->activeDebugSession.has_value() && this->activeDebugSession->waitingForBreak) {
                this->activeDebugSession->connection.writePacket(
                    ResponsePackets::TargetStopped(Signal::TRAP)
                );
                this->activeDebugSession->waitingForBreak = false;
            }

        } catch (const ClientDisconnected&) {
            Logger::info("GDB RSP client disconnected");
            this->activeDebugSession.reset();
            return;

        } catch (const ClientCommunicationError& exception) {
            Logger::error(
                "GDB RSP client communication error - " + exception.getMessage() + " - closing connection"
            );
            this->activeDebugSession.reset();
            return;

        } catch (const DebugServerInterrupted&) {
            // Server was interrupted
            Logger::debug("GDB RSP interrupted");
            return;
        }
    }

    void GdbRspDebugServer::onTargetExecutionResumed(const Events::TargetExecutionResumed&) {
        try {
            if (
                this->activeDebugSession.has_value()
                && this->activeDebugSession->pendingInterrupt
                && this->activeDebugSession->waitingForBreak
            ) {
                Logger::info("Servicing pending interrupt");
                this->targetControllerService.stopTargetExecution();

                this->activeDebugSession->connection.writePacket(
                    ResponsePackets::TargetStopped(Signal::INTERRUPTED)
                );

                this->activeDebugSession->pendingInterrupt = false;
                this->activeDebugSession->waitingForBreak = false;
            }

        } catch (const ClientDisconnected&) {
            Logger::info("GDB RSP client disconnected");
            this->activeDebugSession.reset();
            return;

        } catch (const ClientCommunicationError& exception) {
            Logger::error(
                "GDB RSP client communication error - " + exception.getMessage() + " - closing connection"
            );
            this->activeDebugSession.reset();
            return;

        } catch (const DebugServerInterrupted&) {
            // Server was interrupted
            Logger::debug("GDB RSP interrupted");
            return;
        }
    }
}
