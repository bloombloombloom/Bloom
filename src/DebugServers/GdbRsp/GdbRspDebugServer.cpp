#include "GdbRspDebugServer.hpp"

#include <sys/socket.h>
#include <sys/epoll.h>

#include "src/Logger/Logger.hpp"

#include "Exceptions/ClientDisconnected.hpp"
#include "Exceptions/ClientNotSupported.hpp"
#include "Exceptions/ClientCommunicationError.hpp"
#include "Exceptions/DebugSessionAborted.hpp"
#include "src/Exceptions/Exception.hpp"
#include "src/Exceptions/InvalidConfig.hpp"

namespace Bloom::DebugServers::Gdb
{
    using namespace CommandPackets;
    using namespace ResponsePackets;
    using namespace Exceptions;
    using namespace Bloom::Events;
    using namespace Bloom::Exceptions;

    using Bloom::Targets::TargetRegister;
    using Bloom::Targets::TargetRegisterType;
    using Bloom::Targets::TargetRegisterDescriptor;
    using Bloom::Targets::TargetRegisterDescriptors;
    using Bloom::Targets::TargetBreakpoint;

    void GdbRspDebugServer::init() {
        this->debugServerConfig = GdbDebugServerConfig(DebugServer::debugServerConfig);

        this->socketAddress.sin_family = AF_INET;
        this->socketAddress.sin_port = htons(this->debugServerConfig->listeningPortNumber);

        if (::inet_pton(
                AF_INET,
                this->debugServerConfig->listeningAddress.c_str(),
                &(this->socketAddress.sin_addr)
            ) == 0
        ) {
            // Invalid IP address
            throw InvalidConfig(
                "Invalid IP address provided in config file: (\"" + this->debugServerConfig->listeningAddress
                    + "\")"
            );
        }

        int socketFileDescriptor = 0;

        if ((socketFileDescriptor = ::socket(AF_INET, SOCK_STREAM, 0)) == 0) {
            throw Exception("Failed to create socket file descriptor.");
        }

        if (::setsockopt(
                socketFileDescriptor,
                SOL_SOCKET,
                SO_REUSEADDR,
                &(this->enableReuseAddressSocketOption),
                sizeof(this->enableReuseAddressSocketOption)
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
                + std::to_string(this->debugServerConfig->listeningPortNumber) + ") may be in use.");
        }

        this->serverSocketFileDescriptor = socketFileDescriptor;

        this->eventFileDescriptor = ::epoll_create(2);
        struct epoll_event event = {};
        event.events = EPOLLIN;
        event.data.fd = this->serverSocketFileDescriptor;

        if (::epoll_ctl(this->eventFileDescriptor, EPOLL_CTL_ADD, this->serverSocketFileDescriptor, &event) != 0) {
            throw Exception("Failed epoll_ctl server socket");
        }

        if (this->interruptEventNotifier != nullptr) {
            auto interruptFileDescriptor = this->interruptEventNotifier->getFileDescriptor();
            event.events = EPOLLIN;
            event.data.fd = interruptFileDescriptor;

            if (::epoll_ctl(this->eventFileDescriptor, EPOLL_CTL_ADD, interruptFileDescriptor, &event) != 0) {
                throw Exception("Failed epoll_ctl interrupt event fd");
            }
        }

        Logger::info("GDB RSP address: " + this->debugServerConfig->listeningAddress);
        Logger::info("GDB RSP port: " + std::to_string(this->debugServerConfig->listeningPortNumber));

        this->eventListener->registerCallbackForEventType<Events::TargetControllerStateReported>(
            std::bind(&GdbRspDebugServer::onTargetControllerStateReported, this, std::placeholders::_1)
        );

        this->eventListener->registerCallbackForEventType<Events::TargetExecutionStopped>(
            std::bind(&GdbRspDebugServer::onTargetExecutionStopped, this, std::placeholders::_1)
        );
    }

    void GdbRspDebugServer::close() {
        this->terminateActiveDebugSession();

        if (this->serverSocketFileDescriptor > 0) {
            ::close(this->serverSocketFileDescriptor);
        }
    }

    void GdbRspDebugServer::serve() {
        try {
            if (!this->activeDebugSession.has_value()) {
                Logger::info("Waiting for GDB RSP connection");

                auto connection = this->waitForConnection();

                if (!connection.has_value()) {
                    // Likely an interrupt
                    return;
                }

                connection->accept(this->serverSocketFileDescriptor);
                Logger::info("Accepted GDP RSP connection from " + connection->getIpAddress());

                this->activeDebugSession.emplace(
                    DebugSession(connection.value(), this->getGdbTargetDescriptor())
                );
                EventManager::triggerEvent(std::make_shared<Events::DebugSessionStarted>());

                /*
                 * Before proceeding with a new debug session, we must ensure that the TargetController is able to
                 * service it.
                 */
                if (!this->targetControllerConsole.isTargetControllerInService()) {
                    this->terminateActiveDebugSession();
                    throw DebugSessionAborted("TargetController not in service");
                }
            }

//            auto packets = this->activeDebugSession->connection.readPackets();
//
//            // Only process the last packet - any others will likely be duplicates from an impatient client
//            if (!packets.empty()) {
//                // Double-dispatch to appropriate handler
//                packets.back()->dispatchToHandler(*this);
//            }

        } catch (const ClientDisconnected&) {
            Logger::info("GDB RSP client disconnected");
            this->terminateActiveDebugSession();
            return;

        } catch (const ClientCommunicationError& exception) {
            Logger::error(
                "GDB RSP client communication error - " + exception.getMessage() + " - closing connection"
            );
            this->terminateActiveDebugSession();
            return;

        } catch (const ClientNotSupported& exception) {
            Logger::error("Invalid GDB RSP client - " + exception.getMessage() + " - closing connection");
            this->terminateActiveDebugSession();
            return;

        } catch (const DebugSessionAborted& exception) {
            Logger::warning("GDB debug session aborted - " + exception.getMessage());
            this->terminateActiveDebugSession();
            return;

        } catch (const DebugServerInterrupted&) {
            // Server was interrupted
            Logger::debug("GDB RSP interrupted");
            return;
        }
    }

    std::optional<Connection> GdbRspDebugServer::waitForConnection() {
        if (::listen(this->serverSocketFileDescriptor, 3) != 0) {
            throw Exception("Failed to listen on server socket");
        }

        constexpr int maxEvents = 5;
        std::array<struct epoll_event, maxEvents> events = {};
        int eventCount = ::epoll_wait(
            this->eventFileDescriptor,
            events.data(),
            maxEvents,
            -1
        );

        if (eventCount > 0) {
            for (size_t i = 0; i < eventCount; i++) {
                auto fileDescriptor = events.at(i).data.fd;

                if (fileDescriptor == this->interruptEventNotifier->getFileDescriptor()) {
                    // Interrupted
                    this->interruptEventNotifier->clear();
                    return std::nullopt;
                }
            }

            return Connection(this->interruptEventNotifier);
        }

        return std::nullopt;
    }

    void GdbRspDebugServer::terminateActiveDebugSession() {
        if (this->activeDebugSession.has_value()) {
            this->activeDebugSession->terminate();
            this->activeDebugSession = std::nullopt;

            EventManager::triggerEvent(std::make_shared<Events::DebugSessionFinished>());
        }
    }

    void GdbRspDebugServer::onTargetControllerStateReported(const Events::TargetControllerStateReported& event) {
        if (event.state == TargetControllerState::SUSPENDED && this->activeDebugSession.has_value()) {
            Logger::warning("Terminating debug session - TargetController suspended unexpectedly");
            this->terminateActiveDebugSession();
        }
    }

    void GdbRspDebugServer::onTargetExecutionStopped(const Events::TargetExecutionStopped&) {
        if (this->activeDebugSession.has_value() && this->activeDebugSession->waitingForBreak) {
            this->activeDebugSession->connection.writePacket(TargetStopped(Signal::TRAP));
            this->activeDebugSession->waitingForBreak = false;
        }
    }
}
