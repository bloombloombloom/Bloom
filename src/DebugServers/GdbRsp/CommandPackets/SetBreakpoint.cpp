#include "SetBreakpoint.hpp"

#include <QtCore/QString>

#include "src/DebugServers/GdbRsp/GdbRspDebugServer.hpp"

namespace Bloom::DebugServers::Gdb::CommandPackets
{
    using namespace Bloom::Exceptions;

    void SetBreakpoint::dispatchToHandler(Gdb::GdbRspDebugServer& gdbRspDebugServer) {
        gdbRspDebugServer.handleGdbPacket(*this);
    }

    void SetBreakpoint::init() {
        if (data.size() < 6) {
            throw Exception("Unexpected SetBreakpoint packet size");
        }

        // Z0 = SW breakpoint, Z1 = HW breakpoint
        this->type = (data[1] == 0) ? BreakpointType::SOFTWARE_BREAKPOINT : (data[1] == 1) ?
            BreakpointType::HARDWARE_BREAKPOINT : BreakpointType::UNKNOWN;

        auto packetData = QString::fromLocal8Bit(
            reinterpret_cast<const char*>(this->data.data() + 2),
            static_cast<int>(this->data.size() - 2)
        );

        auto packetSegments = packetData.split(",");
        if (packetSegments.size() < 3) {
            throw Exception("Unexpected number of packet segments in SetBreakpoint packet");
        }

        bool conversionStatus = true;
        this->address = packetSegments.at(1).toUInt(&conversionStatus, 16);

        if (!conversionStatus) {
            throw Exception("Failed to convert address hex value from SetBreakpoint packet.");
        }
    }
}
