#include "Monitor.hpp"

#include "src/DebugServer/Gdb/ResponsePackets/EmptyResponsePacket.hpp"

#include "src/Logger/Logger.hpp"

namespace Bloom::DebugServer::Gdb::CommandPackets
{
    using TargetController::TargetControllerConsole;

    using ResponsePackets::EmptyResponsePacket;

    Monitor::Monitor(const RawPacketType& rawPacket)
        : CommandPacket(rawPacket)
    {
        if (this->data.size() > 6) {
            const auto decodedCommand = Packet::hexToData(
                std::string(this->data.begin() + 6, this->data.end())
            );

            this->command = std::string(decodedCommand.begin(), decodedCommand.end());
            this->command.erase(this->command.find_last_not_of(" ") + 1);
        }
    }

    void Monitor::handle(DebugSession& debugSession, TargetControllerConsole& targetControllerConsole) {
        Logger::error("Unknown custom GDB command (\"" + this->command + "\") received.");
        debugSession.connection.writePacket(EmptyResponsePacket());
    }
}
