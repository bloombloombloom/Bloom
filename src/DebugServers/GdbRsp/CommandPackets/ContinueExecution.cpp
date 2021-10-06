#include "ContinueExecution.hpp"

#include <cstdint>

#include "src/DebugServers/GdbRsp/GdbRspDebugServer.hpp"

using namespace Bloom::DebugServers::Gdb::CommandPackets;

void ContinueExecution::dispatchToHandler(Gdb::GdbRspDebugServer& gdbRspDebugServer) {
    gdbRspDebugServer.handleGdbPacket(*this);
}

void ContinueExecution::init() {
    if (this->data.size() > 1) {
        this->fromProgramCounter = static_cast<std::uint32_t>(
            std::stoi(std::string(this->data.begin(), this->data.end()), nullptr, 16)
        );
    }
}
