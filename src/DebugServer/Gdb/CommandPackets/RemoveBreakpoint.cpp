#include "RemoveBreakpoint.hpp"

#include <QtCore/QString>

#include "src/DebugServer/Gdb/ResponsePackets/OkResponsePacket.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"

#include "src/Targets/TargetBreakpoint.hpp"

#include "src/Logger/Logger.hpp"
#include "src/Exceptions/Exception.hpp"

namespace DebugServer::Gdb::CommandPackets
{
    using Services::TargetControllerService;

    using Targets::TargetBreakpoint;

    using ResponsePackets::OkResponsePacket;
    using ResponsePackets::ErrorResponsePacket;

    using ::Exceptions::Exception;

    RemoveBreakpoint::RemoveBreakpoint(const RawPacket& rawPacket)
        : CommandPacket(rawPacket)
    {
        if (this->data.size() < 6) {
            throw Exception{"Unexpected RemoveBreakpoint packet size"};
        }

        // z0 = SW breakpoint, z1 = HW breakpoint
        this->type = (this->data[1] == '0')
            ? BreakpointType::SOFTWARE_BREAKPOINT
            : (this->data[1] == '1') ? BreakpointType::HARDWARE_BREAKPOINT : BreakpointType::UNKNOWN;

        const auto packetData = QString::fromLocal8Bit(
            reinterpret_cast<const char*>(this->data.data() + 2),
            static_cast<int>(this->data.size() - 2)
        );

        auto packetSegments = packetData.split(",");
        if (packetSegments.size() < 3) {
            throw Exception{"Unexpected number of packet segments in RemoveBreakpoint packet"};
        }

        bool conversionStatus = true;
        this->address = packetSegments.at(1).toUInt(&conversionStatus, 16);

        if (!conversionStatus) {
            throw Exception{"Failed to convert address hex value from RemoveBreakpoint packet."};
        }
    }

    void RemoveBreakpoint::handle(
        DebugSession& debugSession,
        const TargetDescriptor& gdbTargetDescriptor,
        const Targets::TargetDescriptor&,
        TargetControllerService& targetControllerService
    ) {
        Logger::info("Handling RemoveBreakpoint packet");

        try {
            Logger::debug("Removing breakpoint at address " + std::to_string(this->address));

            debugSession.removeExternalBreakpoint(this->address, targetControllerService);
            debugSession.connection.writePacket(OkResponsePacket{});

        } catch (const Exception& exception) {
            Logger::error("Failed to remove breakpoint on target - " + exception.getMessage());
            debugSession.connection.writePacket(ErrorResponsePacket{});
        }
    }
}
