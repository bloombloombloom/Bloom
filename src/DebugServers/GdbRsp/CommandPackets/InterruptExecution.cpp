#include "InterruptExecution.hpp"

#include "src/DebugServers/GdbRsp/GdbRspDebugServer.hpp"

namespace Bloom::DebugServers::Gdb::CommandPackets
{
    void InterruptExecution::dispatchToHandler(Gdb::GdbRspDebugServer& gdbRspDebugServer) {
        gdbRspDebugServer.handleGdbPacket(*this);
    }
}
