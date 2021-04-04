#include <QtCore/QString>

#include "RemoveBreakpoint.hpp"
#include "src/DebugServers/GdbRsp/GdbRspDebugServer.hpp"

using namespace Bloom::DebugServers::Gdb::CommandPackets;
using namespace Bloom::Exceptions;

void RemoveBreakpoint::init() {
    if (data.size() < 6) {
        throw Exception("Unexpected RemoveBreakpoint packet size");
    }

    // z0 = SW breakpoint, z1 = HW breakpoint
    this->type = (data[1] == 0) ? BreakpointType::SOFTWARE_BREAKPOINT : (data[1] == 1) ?
        BreakpointType::HARDWARE_BREAKPOINT : BreakpointType::UNKNOWN;

    auto packetData = QString::fromLocal8Bit(
        reinterpret_cast<const char*>(this->data.data() + 2),
        static_cast<int>(this->data.size() - 2)
    );

    auto packetSegments = packetData.split(",");
    if (packetSegments.size() < 3) {
        throw Exception("Unexpected number of packet segments in RemoveBreakpoint packet");
    }

    bool conversionStatus = true;
    this->address = packetSegments.at(1).toUInt(&conversionStatus, 16);

    if (!conversionStatus) {
        throw Exception("Failed to convert address hex value from RemoveBreakpoint packet.");
    }
}

void RemoveBreakpoint::dispatchToHandler(Gdb::GdbRspDebugServer& gdbRspDebugServer) {
    gdbRspDebugServer.handleGdbPacket(*this);
}
