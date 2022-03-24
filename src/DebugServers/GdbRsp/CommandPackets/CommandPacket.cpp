#include "CommandPacket.hpp"

#include "src/DebugServers/GdbRsp/ResponsePackets/ResponsePacket.hpp"
#include "src/DebugServers/GdbRsp/ResponsePackets/OkResponsePacket.hpp"
#include "src/DebugServers/GdbRsp/ResponsePackets/TargetStopped.hpp"
#include "src/DebugServers/GdbRsp/ResponsePackets/ErrorResponsePacket.hpp"

#include "src/DebugServers/GdbRsp/Signal.hpp"

#include "src/Logger/Logger.hpp"
#include "src/Exceptions/Exception.hpp"

namespace Bloom::DebugServers::Gdb::CommandPackets
{
    using ResponsePackets::ResponsePacket;
    using ResponsePackets::OkResponsePacket;
    using ResponsePackets::TargetStopped;
    using ResponsePackets::ErrorResponsePacket;

    using Exceptions::Exception;

    void CommandPacket::handle(DebugSession& debugSession, TargetControllerConsole& targetControllerConsole) {
        auto packetData = this->getData();
        auto packetString = std::string(packetData.begin(), packetData.end());

        if (packetString[0] == '?') {
            // Status report
            debugSession.connection.writePacket(TargetStopped(Signal::TRAP));

        } else if (packetString[0] == 'D') {
            // Detach packet - there's not really anything we need to do here, so just respond with an OK
            debugSession.connection.writePacket(OkResponsePacket());

        } else if (packetString.find("qAttached") == 0) {
            Logger::debug("Handling qAttached");
            debugSession.connection.writePacket(ResponsePacket({1}));

        } else {
            Logger::debug("Unknown GDB RSP packet: " + packetString + " - returning empty response");

            // Respond with an empty packet
            debugSession.connection.writePacket(ResponsePacket({0}));
        }
    }
}
