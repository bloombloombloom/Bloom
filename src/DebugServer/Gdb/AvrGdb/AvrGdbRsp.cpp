#include "AvrGdbRsp.hpp"

// Command packets
#include "CommandPackets/ReadMemory.hpp"
#include "CommandPackets/WriteMemory.hpp"

namespace Bloom::DebugServer::Gdb::AvrGdb
{
    using namespace Bloom::Exceptions;

    using Bloom::Targets::TargetRegisterDescriptor;
    using Bloom::Targets::TargetRegisterType;

    AvrGdbRsp::AvrGdbRsp(
        const DebugServerConfig& debugServerConfig,
        EventListener& eventListener,
        EventFdNotifier& eventNotifier
    )
        : GdbRspDebugServer(debugServerConfig, eventListener, eventNotifier)
    {}

    void AvrGdbRsp::init() {
        DebugServer::Gdb::GdbRspDebugServer::init();

        this->gdbTargetDescriptor = TargetDescriptor(
            this->targetControllerConsole.getTargetDescriptor()
        );
    }

    std::unique_ptr<Gdb::CommandPackets::CommandPacket> AvrGdbRsp::resolveCommandPacket(
        const RawPacketType& rawPacket
    ) {
        using AvrGdb::CommandPackets::ReadMemory;
        using AvrGdb::CommandPackets::WriteMemory;

        if (rawPacket.size() >= 2) {
            if (rawPacket[1] == 'm') {
                return std::make_unique<ReadMemory>(rawPacket);
            }

            if (rawPacket[1] == 'M') {
                return std::make_unique<WriteMemory>(rawPacket);
            }
        }

        return GdbRspDebugServer::resolveCommandPacket(rawPacket);
    }
}
