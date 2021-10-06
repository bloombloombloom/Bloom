#include "ReadRegisters.hpp"

#include "src/DebugServers/GdbRsp/GdbRspDebugServer.hpp"

using namespace Bloom::DebugServers::Gdb::CommandPackets;

void ReadRegisters::dispatchToHandler(Gdb::GdbRspDebugServer& gdbRspDebugServer) {
    gdbRspDebugServer.handleGdbPacket(*this);
}

void ReadRegisters::init() {
    if (this->data.size() >= 2 && this->data.front() == 'p') {
        // This command packet is requesting a specific register
        this->registerNumber = static_cast<size_t>(std::stoi(std::string(this->data.begin() + 1, this->data.end())));
    }
}
