#include "CommandPacket.hpp"

#include "src/DebugServers/GdbRsp/GdbRspDebugServer.hpp"

namespace Bloom::DebugServers::Gdb::CommandPackets
{
    void CommandPacket::dispatchToHandler(Gdb::GdbRspDebugServer& gdbRspDebugServer) {
        gdbRspDebugServer.handleGdbPacket(*this);
    }
}
