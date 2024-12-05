#include "SetBreakpoint.hpp"

#include <string>

#include "src/DebugServer/Gdb/ResponsePackets/OkResponsePacket.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/EmptyResponsePacket.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"

#include "src/Targets/TargetBreakpoint.hpp"

#include "src/Services/StringService.hpp"

#include "src/Logger/Logger.hpp"
#include "src/Exceptions/Exception.hpp"

namespace DebugServer::Gdb::AvrGdb::CommandPackets
{
    using Services::TargetControllerService;

    using ResponsePackets::OkResponsePacket;
    using ResponsePackets::ErrorResponsePacket;
    using ResponsePackets::EmptyResponsePacket;

    using ::Exceptions::Exception;

    SetBreakpoint::SetBreakpoint(const RawPacket& rawPacket)
        : CommandPacket(rawPacket)
    {
        const auto packetString = std::string{this->data.begin(), this->data.end()};

        if (packetString.size() < 6) {
            throw Exception{"Unexpected SetBreakpoint packet size"};
        }

        // Z0 = SW breakpoint, Z1 = HW breakpoint
        this->type = (packetString[1] == '0')
            ? BreakpointType::SOFTWARE_BREAKPOINT
            : (packetString[1] == '1') ? BreakpointType::HARDWARE_BREAKPOINT : BreakpointType::UNKNOWN;

        const auto firstCommaPos = packetString.find_first_of(',');
        const auto secondCommaPos = packetString.find_first_of(',', firstCommaPos + 1);
        if (firstCommaPos == std::string::npos || secondCommaPos == std::string::npos) {
            throw Exception{"Invalid SetBreakpoint packet"};
        }

        const auto addressString = packetString.substr(firstCommaPos + 1, secondCommaPos - firstCommaPos - 1);
        const auto sizeString = packetString.substr(
            secondCommaPos + 1,
            packetString.find_first_of(';', secondCommaPos) - secondCommaPos - 1
        );

        this->address = Services::StringService::toUint32(addressString, 16);
        this->size = Services::StringService::toUint32(sizeString, 10);
    }

    void SetBreakpoint::handle(
        DebugSession& debugSession,
        const AvrGdbTargetDescriptor& gdbTargetDescriptor,
        const Targets::TargetDescriptor&,
        TargetControllerService& targetControllerService
    ) {
        Logger::info("Handling SetBreakpoint packet");

        try {
            if (this->type == BreakpointType::UNKNOWN) {
                Logger::debug(
                    "Rejecting breakpoint at address " + std::to_string(this->address)
                        + " - unsupported breakpoint type"
                );
                debugSession.connection.writePacket(EmptyResponsePacket{});
                return;
            }

            debugSession.setExternalBreakpoint(
                gdbTargetDescriptor.programAddressSpaceDescriptor,
                gdbTargetDescriptor.programMemorySegmentDescriptor,
                this->address, this->size, targetControllerService
            );
            debugSession.connection.writePacket(OkResponsePacket{});

        } catch (const Exception& exception) {
            Logger::error("Failed to set breakpoint on target - " + exception.getMessage());
            debugSession.connection.writePacket(ErrorResponsePacket{});
        }
    }
}