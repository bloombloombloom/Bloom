#include "CommandPacket.hpp"

#include "src/DebugServer/Gdb/ResponsePackets/ResponsePacket.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/OkResponsePacket.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/TargetStopped.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/EmptyResponsePacket.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"

#include "src/DebugServer/Gdb/Signal.hpp"

#include "src/Logger/Logger.hpp"
#include "src/Exceptions/Exception.hpp"

namespace Bloom::DebugServer::Gdb::CommandPackets
{
    using TargetController::TargetControllerConsole;

    using ResponsePackets::ResponsePacket;
    using ResponsePackets::OkResponsePacket;
    using ResponsePackets::TargetStopped;
    using ResponsePackets::EmptyResponsePacket;
    using ResponsePackets::ErrorResponsePacket;

    using Exceptions::Exception;

    void CommandPacket::handle(DebugSession& debugSession, TargetControllerConsole& targetControllerConsole) {
        const auto packetString = std::string(this->data.begin(), this->data.end());

        if (packetString.empty()) {
            Logger::error("Empty GDB RSP packet received.");
            debugSession.connection.writePacket(ErrorResponsePacket());
            return;
        }

        if (packetString[0] == '?') {
            // Status report
            debugSession.connection.writePacket(TargetStopped(Signal::TRAP));
            return;
        }

        if (packetString.find("vMustReplyEmpty") == 0) {
            Logger::debug("Handling vMustReplyEmpty");
            debugSession.connection.writePacket(EmptyResponsePacket());
            return;
        }

        if (packetString.find("qAttached") == 0) {
            Logger::debug("Handling qAttached");
            debugSession.connection.writePacket(ResponsePacket(std::vector<unsigned char>({1})));
            return;
        }

        Logger::debug("Unknown GDB RSP packet: " + packetString + " - returning empty response");

        // Respond with an empty packet
        debugSession.connection.writePacket(EmptyResponsePacket());
    }
}
