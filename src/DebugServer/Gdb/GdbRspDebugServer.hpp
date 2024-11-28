#pragma once

#include <cstdint>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <queue>
#include <variant>
#include <functional>
#include <optional>
#include <memory>
#include <sys/socket.h>
#include <unistd.h>

#include "src/DebugServer/ServerInterface.hpp"

#include "GdbDebugServerConfig.hpp"
#include "Connection.hpp"
#include "TargetDescriptor.hpp"
#include "DebugSession.hpp"
#include "Signal.hpp"
#include "RegisterDescriptor.hpp"
#include "Feature.hpp"

// Command packets
#include "CommandPackets/CommandPacket.hpp"
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
#include "CommandPackets/VContContinueExecution.hpp"
#include "CommandPackets/VContStepExecution.hpp"

#ifndef EXCLUDE_INSIGHT
#include "CommandPackets/ActivateInsight.hpp"
#endif

// Response packets
#include "ResponsePackets/TargetStopped.hpp"

#include "src/EventManager/EventListener.hpp"
#include "src/Helpers/EpollInstance.hpp"
#include "src/Helpers/EventFdNotifier.hpp"
#include "src/Services/TargetControllerService.hpp"
#include "src/Services/ProcessService.hpp"
#include "src/Services/StringService.hpp"
#include "src/Targets/TargetDescriptor.hpp"
#include "src/Logger/Logger.hpp"

#include "src/EventManager/Events/TargetStateChanged.hpp"
#include "src/EventManager/EventManager.hpp"

#include "Exceptions/ClientDisconnected.hpp"
#include "Exceptions/ClientNotSupported.hpp"
#include "Exceptions/ClientCommunicationError.hpp"
#include "Exceptions/DebugSessionInitialisationFailure.hpp"
#include "Exceptions/DebugServerInterrupted.hpp"

#include "src/Exceptions/Exception.hpp"
#include "src/Exceptions/FatalErrorException.hpp"
#include "src/Exceptions/InvalidConfig.hpp"

namespace DebugServer::Gdb
{
    /**
     * The GdbRspDebugServer is an implementation of the GDB Remote Serial Protocol.
     *
     * This server employs TCP/IP sockets to interface with GDB clients. The listening address and port can be
     * configured in the user's project config file.
     *
     * See https://sourceware.org/gdb/onlinedocs/gdb/Remote-Protocol.html for more info on the GDB Remote Serial
     * Protocol.
     */
    template<
        typename GdbTargetDescriptorType,
        typename DebugSessionType,
        typename CommandPacketType
    >
        requires
            std::is_base_of_v<TargetDescriptor, GdbTargetDescriptorType>
            && std::is_base_of_v<DebugSession, DebugSessionType>
    class GdbRspDebugServer: public ServerInterface
    {
    public:
        explicit GdbRspDebugServer(
            const DebugServerConfig& debugServerConfig,
            const Targets::TargetDescriptor& targetDescriptor,
            GdbTargetDescriptorType&& gdbTargetDescriptor,
            EventListener& eventListener,
            EventFdNotifier& eventNotifier
        )
            : debugServerConfig(GdbDebugServerConfig{debugServerConfig})
            , targetDescriptor(targetDescriptor)
            , gdbTargetDescriptor(std::move(gdbTargetDescriptor))
            , eventListener(eventListener)
            , interruptEventNotifier(eventNotifier)
        {};

        GdbRspDebugServer() = delete;
        virtual ~GdbRspDebugServer() = default;

        GdbRspDebugServer(const GdbRspDebugServer& other) = delete;
        GdbRspDebugServer(GdbRspDebugServer&& other) = delete;

        GdbRspDebugServer& operator = (const GdbRspDebugServer& other) = delete;
        GdbRspDebugServer& operator = (GdbRspDebugServer&& other) = delete;

        /**
         * Prepares the GDB server for listing on the selected address and port.
         */
        void init() override {
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
                throw ::Exceptions::InvalidConfig{
                    "Invalid IP address provided in config file: (\"" + this->debugServerConfig.listeningAddress + "\")"
                };
            }

            const auto socketFileDescriptor = ::socket(AF_INET, SOCK_STREAM, 0);
            if (socketFileDescriptor == 0) {
                throw ::Exceptions::Exception{"Failed to create socket file descriptor."};
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
                throw ::Exceptions::Exception{
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

            this->eventListener.template registerCallbackForEventType<Events::TargetStateChanged>(
                std::bind(&GdbRspDebugServer::onTargetStateChanged, this, std::placeholders::_1)
            );

            if (Services::ProcessService::isManagedByClion()) {
                Logger::warning(
                    "Bloom's process is being managed by CLion - Bloom will automatically shutdown upon detaching from GDB."
                );
            }
        }

        /**
         * Terminates any active debug session and closes the listening socket.
         */
        void close() override {
            this->endDebugSession();

            if (this->serverSocketFileDescriptor.has_value()) {
                ::close(this->serverSocketFileDescriptor.value());
            }
        }

        /**
         * Waits for a connection from a GDB client or services an active one.
         *
         * This function will return when any blocking operation is interrupted via this->interruptEventNotifier.
         */
        void run() override {
            try {
                if (!this->debugSession.has_value()) {
                    Logger::info("Waiting for GDB RSP connection");

                    auto connection = this->waitForConnection();
                    Logger::info("Accepted GDP RSP connection from " + connection.getIpAddress());

                    this->debugSession.emplace(
                        std::move(connection),
                        this->getSupportedFeatures(),
                        this->debugServerConfig
                    );

                    this->targetControllerService.stopTargetExecution();
                    this->targetControllerService.resetTarget();
                }

                const auto commandPacketVariant = this->waitForCommandPacket();

                if (std::holds_alternative<std::unique_ptr<CommandPacketType>>(commandPacketVariant)) {
                    const auto& commandPacket = std::get<std::unique_ptr<CommandPacketType>>(commandPacketVariant);
                    if (!commandPacket) {
                        return;
                    }

                    commandPacket->handle(
                        *(this->debugSession),
                        this->gdbTargetDescriptor,
                        this->targetDescriptor,
                        this->targetControllerService
                    );

                } else {
                    const auto& commandPacket = std::get<std::unique_ptr<CommandPackets::CommandPacket>>(
                        commandPacketVariant
                    );
                    if (!commandPacket) {
                        return;
                    }

                    commandPacket->handle(
                        *(this->debugSession),
                        this->gdbTargetDescriptor,
                        this->targetDescriptor,
                        this->targetControllerService
                    );
                }

            } catch (const Exceptions::ClientDisconnected&) {
                Logger::info("GDB RSP client disconnected");
                this->endDebugSession();
                return;

            } catch (const Exceptions::ClientCommunicationError& exception) {
                Logger::error(
                    "GDB RSP client communication error - " + exception.getMessage() + " - closing connection"
                );
                this->endDebugSession();
                return;

            } catch (const Exceptions::ClientNotSupported& exception) {
                Logger::error("Invalid GDB RSP client - " + exception.getMessage() + " - closing connection");
                this->endDebugSession();
                return;

            } catch (const Exceptions::DebugSessionInitialisationFailure& exception) {
                Logger::warning("GDB debug session initialisation failure - " + exception.getMessage());
                this->endDebugSession();
                return;

            } catch (const Exceptions::DebugServerInterrupted&) {
                // Server was interrupted by an event
                Logger::debug("GDB RSP interrupted");
                return;

            } catch (const ::Exceptions::FatalErrorException& exception) {
                Logger::error("Fatal error occurred - closing connection");
                this->endDebugSession();
                throw exception;
            }
        }

    protected:
        GdbDebugServerConfig debugServerConfig;
        const Targets::TargetDescriptor& targetDescriptor;
        GdbTargetDescriptorType gdbTargetDescriptor;
        EventListener& eventListener;

        /**
         * EventFdNotifier object for interrupting blocking I/O operations.
         *
         * Extracted from this->eventListener.
         *
         * See documentation in src/DebugServer/README.md for more.
         */
        EventFdNotifier& interruptEventNotifier;

        /**
         * When waiting for a connection, we don't listen on the this->serverSocketFileDescriptor directly. Instead,
         * we use an EpollInstance to monitor both this->serverSocketFileDescriptor and this->interruptEventNotifier.
         * This allows us to interrupt any blocking socket IO calls when EventFdNotifier::notify() is called on
         * this->interruptEventNotifier.
         *
         * See GdbRspDebugServer::init()
         * See DebugServer::interruptEventNotifier
         * See EpollInstance
         * See EventFdNotifier
         */
        EpollInstance epollInstance = {};

        Services::TargetControllerService targetControllerService = {};

        struct sockaddr_in socketAddress = {};
        std::optional<int> serverSocketFileDescriptor;

        /**
         * The active debug session.
         */
        std::optional<DebugSessionType> debugSession;

        void endDebugSession() {
            this->debugSession.reset();
        }

        Connection waitForConnection() {
            if (::listen(this->serverSocketFileDescriptor.value(), 3) != 0) {
                throw ::Exceptions::Exception{"Failed to listen on server socket"};
            }

            const auto eventFileDescriptor = this->epollInstance.waitForEvent();
            if (
                !eventFileDescriptor.has_value()
                || *eventFileDescriptor == this->interruptEventNotifier.getFileDescriptor()
            ) {
                this->interruptEventNotifier.clear();
                throw Exceptions::DebugServerInterrupted{};
            }

            return {this->serverSocketFileDescriptor.value(), this->interruptEventNotifier};
        }

        /**
         * Waits for a command packet from the connected GDB client.
         *
         * This function will first attempt to construct a server-implementation-specific command packet, but if that
         * yields nothing, it will fall back to a generic command packet.
         *
         * @return
         */
        std::variant<
            std::unique_ptr<CommandPacketType>,
            std::unique_ptr<CommandPackets::CommandPacket>
        > waitForCommandPacket() {
            const auto rawPackets = this->debugSession->connection.readRawPackets();
            assert(!rawPackets.empty());

            if (rawPackets.size() > 1) {
                const auto& firstRawPacket = rawPackets.front();

                if (firstRawPacket.size() == 5 && firstRawPacket[1] == 0x03) {
                    // Interrupt packet that came in too quickly before another packet
                    this->debugSession->pendingInterrupt = true;

                } else {
                    Logger::warning("Multiple packets received from GDB - only the most recent will be processed");
                }
            }

            auto commandPacket = this->rawPacketToCommandPacket(rawPackets.back());
            if (commandPacket) {
                return commandPacket;
            }

            return this->rawPacketToGenericCommandPacket(rawPackets.back());
        }

        /**
         * Should construct a server-implementation-specific command packet from a raw packet.
         *
         * This function should return a nullptr if the server implementation does not recognise/handle the command.
         *
         * @param rawPacket
         * @return
         */
        virtual std::unique_ptr<CommandPacketType> rawPacketToCommandPacket(const RawPacket& rawPacket) = 0;

        /**
         * If rawPacketToCommandPacket() returns a nullptr, we'll attempt to construct a generic command packet.
         *
         * @param rawPacket
         * @return
         */
        std::unique_ptr<CommandPackets::CommandPacket> rawPacketToGenericCommandPacket(const RawPacket& rawPacket) {
            if (rawPacket.size() < 2) {
                throw ::Exceptions::Exception{"Invalid raw packet - no data"};
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

            if (rawPacketString.find("vCont;c") == 0 || rawPacketString.find("vCont;C") == 0) {
                return std::make_unique<CommandPackets::VContContinueExecution>(rawPacket);
            }

            if (rawPacketString.find("vCont;s") == 0 || rawPacketString.find("vCont;S") == 0) {
                return std::make_unique<CommandPackets::VContStepExecution>(rawPacket);
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

            return std::make_unique<CommandPackets::CommandPacket>(rawPacket);
        }

        /**
         * Should return a set of GDB features supported by the AVR GDB server. Each supported feature may come with an
         * optional value.
         *
         * The set of features returned by this function will be stored against the active debug session object.
         *
         * @return
         */
        virtual std::set<std::pair<Feature, std::optional<std::string>>> getSupportedFeatures() = 0;

        void onTargetStateChanged(const Events::TargetStateChanged& event) {
            using Targets::TargetExecutionState;

            if (!this->debugSession.has_value()) {
                return;
            }

            if (event.newState.executionState == event.previousState.executionState) {
                // Execution state hasn't changed. Probably just a mode change. Ignore...
                return;
            }

            const auto executionState = event.newState.executionState.load();

            try {
                if (executionState == TargetExecutionState::STOPPED && this->debugSession->waitingForBreak) {
                    this->handleTargetStoppedGdbResponse(event.newState.programCounter.load().value());
                    return;
                }

                if (
                    executionState == TargetExecutionState::RUNNING
                    || executionState == TargetExecutionState::STEPPING
                ) {
                    this->handleTargetResumedGdbResponse();
                    return;
                }

            } catch (const Exceptions::ClientDisconnected&) {
                Logger::info("GDB RSP client disconnected");
                this->endDebugSession();
                return;

            } catch (const Exceptions::ClientCommunicationError& exception) {
                Logger::error(
                    "GDB RSP client communication error - " + exception.getMessage() + " - closing connection"
                );
                this->endDebugSession();
                return;

            } catch (const Exceptions::DebugServerInterrupted&) {
                // Server was interrupted
                Logger::debug("GDB RSP interrupted");
                return;

            } catch (const ::Exceptions::FatalErrorException& exception) {
                this->endDebugSession();
                throw exception;

            } catch (const ::Exceptions::Exception& exception) {
                Logger::error("Failed to handle target execution state changed event - " + exception.getMessage());
            }
        }

        virtual void handleTargetStoppedGdbResponse(Targets::TargetMemoryAddress programAddress) {
            if (this->debugSession->activeRangeSteppingSession.has_value()) {
                this->debugSession->terminateRangeSteppingSession(this->targetControllerService);
            }

            this->debugSession->connection.writePacket(ResponsePackets::TargetStopped{Signal::TRAP});
            this->debugSession->waitingForBreak = false;
        }

        virtual void handleTargetResumedGdbResponse() {
            if (this->debugSession->waitingForBreak && this->debugSession->pendingInterrupt) {
                Logger::info("Servicing pending interrupt");
                this->targetControllerService.stopTargetExecution();

                if (this->debugSession->activeRangeSteppingSession.has_value()) {
                    this->debugSession->terminateRangeSteppingSession(this->targetControllerService);
                }

                this->debugSession->connection.writePacket(ResponsePackets::TargetStopped{Signal::INTERRUPTED});
                this->debugSession->pendingInterrupt = false;
                this->debugSession->waitingForBreak = false;
            }
        }
    };
}
