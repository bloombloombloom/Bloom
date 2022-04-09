#pragma once

#include <vector>
#include <memory>

#include "src/DebugServer/Gdb/Packet.hpp"
#include "src/DebugServer/Gdb/DebugSession.hpp"

#include "src/TargetController/TargetControllerConsole.hpp"

namespace Bloom::DebugServer::Gdb::CommandPackets
{
    /**
     * GDB RSP command packets are sent to the server, from the GDB client. These packets carry instructions that the
     * server is expected to carry out. Upon completion, the server is expected to respond to the client with
     * a ResponsePacket.
     *
     * For some command packets, we define a specific data structure by extending this CommandPacket class. These
     * classes extend the data structure to include fields for data which may be specific to the command.
     * They also implement additional methods that allow us to easily access the additional data. An example
     * of this would be the SupportedFeaturesQuery class. It extends the CommandPacket class and provides access
     * to additional data fields that are specific to the command (in this case, a set of GDB features reported to be
     * supported by the GDB client).
     *
     * Typically, command packets that require specific data structures are handled in a dedicated handler method
     * in the GdbRspDebugServer. This is done by double dispatching the packet object to the appropriate handler.
     * See CommandPacket::dispatchToHandler(), GdbRspDebugServer::serve() and the overloads
     * for GdbRspDebugServer::handleGdbPacket() for more on this.
     *
     * Some command packets are so simple they do not require a dedicated data structure. An example of this is
     * the halt reason packet, which contains nothing more than an ? character in the packet body. These packets are
     * typically handled in the generic GdbRspDebugServer::handleGdbPacket(CommandPacket&) method.
     *
     * See the Packet class for information on how the raw packets are formatted.
     */
    class CommandPacket: public Packet
    {
    public:
        explicit CommandPacket(const RawPacketType& rawPacket): Packet(rawPacket) {}

        /**
         * Should handle the command for the current active debug session.
         *
         * @param debugSession
         *  The current active debug session.
         *
         * @param targetControllerConsole
         */
        virtual void handle(
            DebugSession& debugSession,
            TargetController::TargetControllerConsole& targetControllerConsole
        );
    };
}
