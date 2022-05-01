#pragma once

#include <cstdint>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <queue>
#include <optional>

#include "src/DebugServer/ServerInterface.hpp"

#include "GdbDebugServerConfig.hpp"
#include "src/EventManager/EventListener.hpp"
#include "src/Helpers/EpollInstance.hpp"
#include "src/Helpers/EventFdNotifier.hpp"
#include "src/TargetController/TargetControllerConsole.hpp"

#include "Connection.hpp"
#include "TargetDescriptor.hpp"
#include "DebugSession.hpp"
#include "Signal.hpp"
#include "RegisterDescriptor.hpp"
#include "Feature.hpp"
#include "CommandPackets/CommandPacket.hpp"

#include "src/EventManager/Events/TargetControllerStateChanged.hpp"
#include "src/EventManager/Events/TargetExecutionStopped.hpp"

namespace Bloom::DebugServer::Gdb
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
    class GdbRspDebugServer: public ServerInterface
    {
    public:
        explicit GdbRspDebugServer(
            const DebugServerConfig& debugServerConfig,
            EventListener& eventListener,
            EventFdNotifier& eventNotifier
        );

        GdbRspDebugServer() = delete;
        virtual ~GdbRspDebugServer() = default;

        GdbRspDebugServer(const GdbRspDebugServer& other) = delete;
        GdbRspDebugServer(GdbRspDebugServer&& other) = delete;

        GdbRspDebugServer& operator = (const GdbRspDebugServer& other) = delete;
        GdbRspDebugServer& operator = (GdbRspDebugServer&& other) = delete;

        [[nodiscard]] std::string getName() const override {
            return "GDB Remote Serial Protocol DebugServer";
        };

        /**
         * Prepares the GDB server for listing on the selected address and port.
         */
        void init() override;

        /**
         * Terminates any active debug session and closes the listening socket.
         */
        void close() override;

        /**
         * Waits for a connection from a GDB client or services an active one.
         *
         * This function will return when any blocking operation is interrupted via this->interruptEventNotifier.
         */
        void run() override;

    protected:
        /**
         * User project configuration specific to the GDB RSP debug server.
         */
        GdbDebugServerConfig debugServerConfig;

        /**
         * The DebugServerComponent's event listener.
         */
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
        EpollInstance epollInstance = EpollInstance();

        /**
         * Passed to command handlers (see CommandPacket::handle()).
         *
         * See documentation in src/DebugServer/Gdb/README.md for more on how GDB commands are processed.
         */
        TargetController::TargetControllerConsole targetControllerConsole = TargetController::TargetControllerConsole();

        /**
         * Listening socket address
         */
        struct sockaddr_in socketAddress = {};

        /**
         * Listening socket file descriptor
         */
        std::optional<int> serverSocketFileDescriptor;

        /**
         * When a connection with a GDB client is established, a new instance of the DebugSession class is created and
         * held here. A value of std::nullopt means there is no active debug session present.
         */
        std::optional<DebugSession> activeDebugSession;

        /**
         * Waits for a GDB client to connect on the listening socket.
         *
         * This function may return an std::nullopt, if the waiting was interrupted by some other event.
         */
        std::optional<Connection> waitForConnection();

        /**
         * Waits for a command packet from the connected GDB client.
         *
         * @return
         */
        std::unique_ptr<CommandPackets::CommandPacket> waitForCommandPacket();

        /**
         * Should construct a derived instance of the CommandPackets::CommandPacket class, from a raw packet.
         *
         * Derived implementations of this GDB server class can override this function to include support for more
         * specific GDB commands. For example, the AVR GDB implementation overrides this function to support AVR
         * specific implementations of the read and write memory GDB commands.
         *
         * This function should never return a nullptr. If the command is unknown, it should simply return an
         * instance of the CommandPackets::CommandPacket base class. The handler (CommandPacket::handle()) for the base
         * class will handle unknown packets by responding with an empty response packet (as is expected by the GDB
         * client).
         *
         * @param rawPacket
         * @return
         */
        virtual std::unique_ptr<CommandPackets::CommandPacket> resolveCommandPacket(const RawPacketType& rawPacket);

        /**
         * Terminates any active debug session (if any) by closing the connection to the GDB client.
         */
        void terminateActiveDebugSession();

        /**
         * Should return the GDB target descriptor for the connected target.
         *
         * NOTE: This function returns a target descriptor specific to GDB and the target. It's not the same data
         * struct as Targets::TargetDescriptor. But the GDB target descriptor does hold an instance to
         * Targets::TargetDescriptor. See the Gdb::TargetDescriptor::targetDescriptor class member.
         *
         * @return
         */
        virtual const TargetDescriptor& getGdbTargetDescriptor() = 0;

        /**
         * Responds to any unexpected TargetController state changes.
         *
         * @param event
         */
        void onTargetControllerStateChanged(const Events::TargetControllerStateChanged& event);

        /**
         * If the GDB client is currently waiting for the target execution to stop, this event handler will issue
         * a "stop reply" packet to the client once the target execution stops.
         */
        void onTargetExecutionStopped(const Events::TargetExecutionStopped&);
    };
}
