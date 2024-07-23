#pragma once

#include <cstdint>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <queue>
#include <optional>
#include <memory>

#include "src/DebugServer/ServerInterface.hpp"

#include "GdbDebugServerConfig.hpp"
#include "src/EventManager/EventListener.hpp"
#include "src/Helpers/EpollInstance.hpp"
#include "src/Helpers/EventFdNotifier.hpp"
#include "src/Services/TargetControllerService.hpp"
#include "src/Targets/TargetDescriptor.hpp"

#include "Connection.hpp"
#include "TargetDescriptor.hpp"
#include "DebugSession.hpp"
#include "Signal.hpp"
#include "RegisterDescriptor.hpp"
#include "Feature.hpp"
#include "CommandPackets/CommandPacket.hpp"

#include "src/EventManager/Events/TargetStateChanged.hpp"

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
    class GdbRspDebugServer: public ServerInterface
    {
    public:
        explicit GdbRspDebugServer(
            const DebugServerConfig& debugServerConfig,
            const Targets::TargetDescriptor& targetDescriptor,
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
        GdbDebugServerConfig debugServerConfig;

        const Targets::TargetDescriptor& targetDescriptor;

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
        Services::TargetControllerService targetControllerService = Services::TargetControllerService();

        /**
         * Listening socket address
         */
        struct sockaddr_in socketAddress = {};

        /**
         * Listening socket file descriptor
         */
        std::optional<int> serverSocketFileDescriptor;

        /**
         * Should start a new debug session for the newly established `connection`.
         *
         * @param connection
         * @return
         */
        virtual DebugSession* startDebugSession(Connection&& connection) = 0;

        /**
         * Should end the currently active debug session, if one exists.
         */
        virtual void endDebugSession() = 0;

        /**
         * Should return a non-owning pointer of the currently active debug session. Or nullptr if there is no active
         * session.
         *
         * @return
         */
        virtual DebugSession* getActiveDebugSession() = 0;

        /**
         * Waits for a GDB client to connect on the listening socket.
         */
        Connection waitForConnection();

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
        virtual std::unique_ptr<CommandPackets::CommandPacket> resolveCommandPacket(const RawPacket& rawPacket);

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

        void onTargetStateChanged(const Events::TargetStateChanged& event);
        virtual void handleTargetStoppedGdbResponse(Targets::TargetMemoryAddress programAddress);
        virtual void handleTargetResumedGdbResponse();
    };
}
