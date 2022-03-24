#pragma once

#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <queue>
#include <cstdint>

#include "src/DebugServers/DebugServer.hpp"

#include "GdbDebugServerConfig.hpp"

#include "Connection.hpp"
#include "TargetDescriptor.hpp"
#include "DebugSession.hpp"
#include "Signal.hpp"
#include "RegisterDescriptor.hpp"
#include "Feature.hpp"
#include "CommandPackets/CommandPacketFactory.hpp"

#include "src/Helpers/EventNotifier.hpp"
#include "src/Helpers/BiMap.hpp"

#include "src/Targets/TargetRegister.hpp"

// Response packets
#include "ResponsePackets/SupportedFeaturesResponse.hpp"
#include "ResponsePackets/TargetStopped.hpp"

namespace Bloom::DebugServers::Gdb
{
    /**
     * The GdbRspDebugServer is an implementation of a GDB server using the GDB Remote Serial Protocol.
     *
     * This DebugServer employs TCP/IP sockets to interface with GDB clients. The listening address can be configured
     * in the user's project config file.
     *
     * See https://sourceware.org/gdb/onlinedocs/gdb/Remote-Protocol.html for more info on the GDB Remote
     * Serial Protocol.
     *
     * @TODO: This could do with some cleaning.
     */
    class GdbRspDebugServer: public DebugServer
    {
    public:
        explicit GdbRspDebugServer(
            const ProjectConfig& projectConfig,
            const EnvironmentConfig& environmentConfig,
            const DebugServerConfig& debugServerConfig
        ): DebugServer(projectConfig, environmentConfig, debugServerConfig) {};

        std::string getName() const override {
            return "GDB Remote Serial Protocol DebugServer";
        };

    protected:
        std::optional<GdbDebugServerConfig> debugServerConfig;

        /**
         * Listening socket address
         */
        struct sockaddr_in socketAddress = {};

        /**
         * Listening socket file descriptor
         */
        int serverSocketFileDescriptor = -1;

        /**
         * We don't listen on the this->serverSocketFileDescriptor directly. Instead, we add it to an epoll set, along
         * with the this->interruptEventNotifier FD. This allows us to interrupt any blocking socket IO calls when
         * we have other things to do.
         *
         * See GdbRspDebugServer::init()
         * See DebugServer::interruptEventNotifier
         * See EventNotifier
         */
        int eventFileDescriptor = -1;

        /**
         * SO_REUSEADDR option value for listening socket.
         */
        int enableReuseAddressSocketOption = 1;

        std::optional<DebugSession> activeDebugSession;

        /**
         * Prepares the GDB server for listing on the selected address and port.
         */
        void init() override;

        /**
         * Closes any client connection as well as the listening socket file descriptor.
         */
        void close() override;

        /**
         * See DebugServer::serve()
         */
        void serve() override;

        /**
         * Waits for a GDB client to connect on the listening socket.
         *
         * This function may return an std::nullopt, if the waiting was interrupted by some other event.
         */
        std::optional<Connection> waitForConnection();

        void terminateActiveDebugSession();

        virtual const TargetDescriptor& getGdbTargetDescriptor() = 0;


        void onTargetControllerStateReported(const Events::TargetControllerStateReported& event);

        /**
         * If the GDB client is currently waiting for the target execution to stop, this event handler will issue
         * a "stop reply" packet to the client once the target execution stops.
         */
        void onTargetExecutionStopped(const Events::TargetExecutionStopped&);
    };
}
