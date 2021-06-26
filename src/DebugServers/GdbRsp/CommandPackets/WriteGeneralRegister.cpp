#include "WriteGeneralRegister.hpp"
#include "src/DebugServers/GdbRsp/GdbRspDebugServer.hpp"
#include "src/Exceptions/Exception.hpp"

using namespace Bloom::DebugServers::Gdb::CommandPackets;
using namespace Bloom::Exceptions;

void WriteGeneralRegister::init() {
    // The P packet updates a single register
    auto packet = std::string(this->data.begin(), this->data.end());

    if (packet.size() < 4) {
        throw Exception("Invalid P command packet - insufficient data in packet.");
    }

    if (packet.find('=') == std::string::npos) {
        throw Exception("Invalid P command packet - unexpected format");
    }

    auto packetSegments = QString::fromStdString(packet).split("=");
    this->registerNumber = static_cast<int>(packetSegments.front().midRef(1).toUInt(nullptr, 16));
    this->registerValue = Packet::hexToData(packetSegments.back().toStdString());
    std::reverse(this->registerValue.begin(), this->registerValue.end());
}

void WriteGeneralRegister::dispatchToHandler(Gdb::GdbRspDebugServer& gdbRspDebugServer) {
    gdbRspDebugServer.handleGdbPacket(*this);
}
