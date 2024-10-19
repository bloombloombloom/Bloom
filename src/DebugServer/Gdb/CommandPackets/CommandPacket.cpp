#include "CommandPacket.hpp"

#include "src/DebugServer/Gdb/ResponsePackets/ResponsePacket.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/OkResponsePacket.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/TargetStopped.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/EmptyResponsePacket.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"

#include "src/DebugServer/Gdb/Signal.hpp"

#include "src/DebugServer/Gdb/Exceptions/ClientCommunicationError.hpp"
#include "src/Logger/Logger.hpp"

namespace DebugServer::Gdb::CommandPackets
{
    using Services::TargetControllerService;

    using ResponsePackets::ResponsePacket;
    using ResponsePackets::OkResponsePacket;
    using ResponsePackets::TargetStopped;
    using ResponsePackets::EmptyResponsePacket;
    using ResponsePackets::ErrorResponsePacket;

    CommandPacket::CommandPacket(const RawPacket& rawPacket) {
        if (rawPacket.size() < 5) {
            throw Exceptions::ClientCommunicationError{"Invalid raw packet size"};
        }

        this->data.insert(this->data.begin(), rawPacket.begin() + 1, rawPacket.end() - 3);
    }

    void CommandPacket::handle(
        DebugSession& debugSession,
        const TargetDescriptor&,
        const Targets::TargetDescriptor&,
        TargetControllerService&
    ) {
        const auto packetString = std::string{this->data.begin(), this->data.end()};

        if (packetString.empty()) {
            Logger::error("Empty GDB RSP packet received.");
            debugSession.connection.writePacket(ErrorResponsePacket{});
            return;
        }

        if (packetString[0] == '?') {
            // Status report
            debugSession.connection.writePacket(TargetStopped{Signal::TRAP});
            return;
        }

        if (packetString.find("vMustReplyEmpty") == 0) {
            Logger::info("Handling vMustReplyEmpty");
            debugSession.connection.writePacket(EmptyResponsePacket{});
            return;
        }

        if (packetString.find("qAttached") == 0) {
            Logger::info("Handling qAttached");
            debugSession.connection.writePacket(ResponsePacket{std::vector<unsigned char>({1})});
            return;
        }

        if (!debugSession.serverConfig.packetAcknowledgement && packetString.find("QStartNoAckMode") == 0) {
            Logger::info("Handling QStartNoAckMode");
            /*
             * We respond to the command before actually disabling packet acknowledgement, because GDB will send one
             * final acknowledgement byte to acknowledge the response.
             */
            debugSession.connection.writePacket(OkResponsePacket{});
            debugSession.connection.packetAcknowledgement = false;
            return;
        }

        Logger::debug("Unknown GDB RSP packet: " + packetString + " - returning empty response");

        // GDB expects an empty response for all unsupported commands
        debugSession.connection.writePacket(EmptyResponsePacket{});
    }
}
