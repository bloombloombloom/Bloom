#include "ReadMemory.hpp"
#include "src/DebugServers/GdbRsp/GdbRspDebugServer.hpp"

using namespace Bloom::DebugServers::Gdb::CommandPackets;
using namespace Bloom::Exceptions;

void ReadMemory::init() {
    if (this->data.size() < 4) {
        throw Exception("Invalid packet length");
    }

    auto packetString = QString::fromLocal8Bit(
        reinterpret_cast<const char*>(this->data.data() + 1),
        static_cast<int>(this->data.size() - 1)
    );

    /*
     * The read memory ('m') packet consists of two segments, an address and a number of bytes to read.
     * These are separated by a comma character.
     */
    auto packetSegments = packetString.split(",");

    if (packetSegments.size() != 2) {
        throw Exception("Unexpected number of segments in packet data: " + std::to_string(packetSegments.size()));
    }

    bool conversionStatus = false;
    this->startAddress = packetSegments.at(0).toUInt(&conversionStatus, 16);

    if (!conversionStatus) {
        throw Exception("Failed to parse start address from read memory packet data");
    }

    this->bytes = packetSegments.at(1).toUInt(&conversionStatus, 16);

    if (!conversionStatus) {
        throw Exception("Failed to parse read length from read memory packet data");
    }
}

void ReadMemory::dispatchToHandler(Gdb::GdbRspDebugServer& gdbRspDebugServer) {
    gdbRspDebugServer.handleGdbPacket(*this);
}
