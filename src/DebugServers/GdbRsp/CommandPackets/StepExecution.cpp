#include <cstdint>

#include "src/DebugServers/GdbRsp/GdbRspDebugServer.hpp"
#include "StepExecution.hpp"

using namespace Bloom::DebugServers::Gdb::CommandPackets;

void StepExecution::dispatchToHandler(Gdb::GdbRspDebugServer& gdbRspDebugServer) {
    gdbRspDebugServer.handleGdbPacket(*this);
}

void StepExecution::init() {
    if (this->data.size() > 1) {
        this->fromProgramCounter = static_cast<std::uint32_t>(
            std::stoi(std::string(this->data.begin(), this->data.end()), nullptr, 16)
        );
    }
}
