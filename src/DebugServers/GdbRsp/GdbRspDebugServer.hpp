#pragma once

#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <queue>
#include <cstdint>

#include "../DebugServer.hpp"
#include "Connection.hpp"
#include "Signal.hpp"
#include "RegisterDescriptor.hpp"
#include "Feature.hpp"
#include "src/Helpers/EventNotifier.hpp"
#include "src/Helpers/BiMap.hpp"
#include "CommandPackets/CommandPacketFactory.hpp"
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
        explicit GdbRspDebugServer(EventManager& eventManager): DebugServer(eventManager) {};

        std::string getName() const override {
            return "GDB Remote Serial Protocol DebugServer";
        };

        /**
         * Handles any other GDB command packet that has not been promoted to a more specific type.
         * This would be packets like "?" and "qAttached".
         *
         * @param packet
         */
        virtual void handleGdbPacket(CommandPackets::CommandPacket& packet);

        /**
         * Handles the supported features query ("qSupported") command packet.
         *
         * @param packet
         */
        virtual void handleGdbPacket(CommandPackets::SupportedFeaturesQuery& packet);

        /**
         * Handles the read registers ("g" and "p") command packet.
         *
         * @param packet
         */
        virtual void handleGdbPacket(CommandPackets::ReadRegisters& packet);

        /**
         * Handles the write general register ("P") command packet.
         *
         * @param packet
         */
        virtual void handleGdbPacket(CommandPackets::WriteRegister& packet);

        /**
         * Handles the continue execution ("c") command packet.
         *
         * @param packet
         */
        virtual void handleGdbPacket(CommandPackets::ContinueExecution& packet);

        /**
         * Handles the step execution ("s") packet.
         *
         * @param packet
         */
        virtual void handleGdbPacket(CommandPackets::StepExecution& packet);

        /**
         * Handles the read memory ("m") command packet.
         *
         * @param packet
         */
        virtual void handleGdbPacket(CommandPackets::ReadMemory& packet);

        /**
         * Handles the write memory ("M") command packet.
         *
         * @param packet
         */
        virtual void handleGdbPacket(CommandPackets::WriteMemory& packet);

        /**
         * Handles the set breakpoint ("Z") command packet.
         *
         * @param packet
         */
        virtual void handleGdbPacket(CommandPackets::SetBreakpoint& packet);

        /**
         * Handles the remove breakpoint ("z") command packet.
         *
         * @param packet
         */
        virtual void handleGdbPacket(CommandPackets::RemoveBreakpoint& packet);

        /**
         * Handles the interrupt command packet.
         * Will attempt to halt execution on the target. Should respond with a "stop reply" packet, or an error code.
         *
         * @param packet
         */
        virtual void handleGdbPacket(CommandPackets::InterruptExecution& packet);

    protected:
        /**
         * The port number for the GDB server to listen on.
         *
         * This will be pulled from the user's project configuration, if set. Otherwise it will default to whatever is
         * set here.
         */
        std::uint16_t listeningPortNumber = 1442;

        /**
         * The address for the GDB server to listen on.
         *
         * Like the port number, this can also be pulled from the user's project configuration.
         */
        std::string listeningAddress = "127.0.0.1";

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

        /**
         * The current active GDB client connection, if any.
         */
        std::optional<Connection> clientConnection;

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
         * Waits for a GDB client to connect on the listening socket. Accepts the connection and
         * sets this->clientConnection.
         */
        void waitForConnection();

        void closeClientConnection() {
            if (this->clientConnection.has_value()) {
                this->clientConnection->close();
                this->clientConnection = std::nullopt;
                this->eventManager.triggerEvent(std::make_shared<Events::DebugSessionFinished>());
            }
        }

        /**
         * GDB clients encode memory type information (flash, ram, eeprom, etc) in memory addresses. This is typically
         * hardcoded in the GDB client source. This method extracts memory type information from a given memory address.
         * The specifics of the encoding may vary with targets, which is why this method is virtual. For an example,
         * see the implementation of this method in AvrGdbRsp.
         *
         * @param address
         * @return
         */
        virtual Targets::TargetMemoryType getMemoryTypeFromGdbAddress(std::uint32_t address) = 0;

        /**
         * Removes memory type information from memory address.
         * See comment for GdbRspDebugServer::getMemoryTypeFromGdbAddress()
         *
         * @param address
         * @return
         */
        virtual std::uint32_t removeMemoryTypeIndicatorFromGdbAddress(std::uint32_t address) = 0;

        /**
         * Should return the mapping of GDB register numbers to GDB register descriptors.
         */
        virtual const BiMap<GdbRegisterNumberType, RegisterDescriptor>& getRegisterNumberToDescriptorMapping() = 0;

        /**
         * Should retrieve the GDB register number, given a target register descriptor. Or std::nullopt if the target
         * register descriptor isn't mapped to any GDB register.
         *
         * @param registerDescriptor
         * @return
         */
        virtual std::optional<GdbRegisterNumberType> getRegisterNumberFromTargetRegisterDescriptor(
            const Targets::TargetRegisterDescriptor& registerDescriptor
        ) = 0;

        /**
         * Should retrieve the GDB register descriptor for a given GDB register number.
         *
         * @param number
         * @return
         */
        virtual const RegisterDescriptor& getRegisterDescriptorFromNumber(GdbRegisterNumberType number) = 0;

        /**
         * Should retrieve the mapped target register descriptor for a given GDB register number.
         *
         * @param number
         * @return
         */
        virtual const Targets::TargetRegisterDescriptor& getTargetRegisterDescriptorFromNumber(
            GdbRegisterNumberType number
        ) = 0;

        void onTargetControllerStateReported(const Events::TargetControllerStateReported& event);

        /**
         * If the GDB client is currently waiting for the target execution to stop, this event handler will issue
         * a "stop reply" packet to the client once the target execution stops.
         */
        void onTargetExecutionStopped(const Events::TargetExecutionStopped&);
    };
}
